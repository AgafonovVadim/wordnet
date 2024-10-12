### Wordnet [![CI](https://github.com/AgafonovVadim/wordnet/actions/workflows/tests.yml/badge.svg)](https://github.com/AgafonovVadim/wordnet/actions/workflows/tests.yml)

WordNet is a specially structured database of English words. It is used in various computer linguistics and cognitive science tasks. One example of its use is the IBM computer system Watson, developed for quickly finding answers in the game "Jeopardy!".

Here's how the WordNet database is organized:

The entire set of words in the database is divided into subsets of synonyms (synsets). A synset is a set of words that represent the same concept. Between some synsets, a semantic relationship of one of two types can be established:
1. Concept A is a hyponym of concept B, i.e., concept A expresses a particular case of concept B.
2. Concept A is a hypernym of concept B, i.e., concept B expresses a particular case of concept A.

Let's define the set of vertices ```V = {s | s is a synset}``` and the set of edges ```E = {(a -> b) | a, b are synsets; a is a hyponym of b}```. Then we can build a graph ```G = (V, E)```. G is a directed acyclic graph with one root vertex.

Here's an example of such a graph:

![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-event.png)

With the help of the resulting graph, we can solve the following two tasks.

## Tasks

1. Determining the semantic closeness of two given words A and B. The semantic closeness of two words is the closeness of the concepts that these words express. The task is complicated by the fact that the same word can express several concepts.

2. Identifying the "extra" word from a given list of words. An "extra" word can be defined as a word that is least semantically related to the other words.

To solve these tasks, we'll introduce the following concepts.

### Necessary Concepts

1. Common ancestor for two vertices v and w is a vertex that is reachable from v and w. Since the WordNet graph has one root vertex, a common ancestor exists for any two vertices.

2. Path from vertex v to vertex w through common ancestor x (ancestral path). This is a path consisting of two parts: the path from v to x and the path from w to x.

3. Path length is the number of edges belonging to the given path.

4. Shortest path from vertex v to w through common ancestor. This is the path from vertex v to vertex w through common ancestor of minimum length.

5. Shortest common ancestor. This is a common ancestor x that lies on the shortest path from vertex v to w.
![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-sca.png)

6. Shortest common ancestor for two sets of vertices A and B. Between pairs of vertices ```{(a, b) | a from A, b from B}``` we can build a set of shortest paths and select the path P of minimum length. Then the shortest common ancestor for two sets of vertices A and B is the shortest common ancestor of path P.
![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-sca-set.png)

### Solution for Task 1
For two words x and y, let's define
```
A = set of all synsets containing word x
B = set of all synsets containing word y
```
and calculate
```
distance(x, y) = length of the shortest path between sets A and B
sca(x, y) = shortest common ancestor between A and B
```
![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-distance.png)

### Solution for Task 2
Given a set of words ```X = {x_i | i = 1..n}```. For each word x_i, let's define its distance to the set of the remaining words ```R_i = {x_k | k = 1..n, k != i}``>:

```d_i = distance(x_i, x_1) + distance(x_i, x_2) + ... + distance(x_i, x_n)```

Then the solution to the task is the word that has the maximum distance to the set of the remaining words.

## Implementation in C++

To solve the tasks, we'll need to implement the following classes:

### 1. WordNet class for storing data
```
class WordNet
{
public:
    WordNet(const std::istream & synsets, const std::istream & hypernyms);

    /**
     * Simple proxy class used to enumerate nouns.
     *
     * Usage example:
     *
     * WordNet wordnet{...};
     * ...
     * for (const std::string & noun : wordnet.nouns()) {
     *     // ...
     * }
     */
    class Nouns
    {
        class iterator
        {
            using iterator_category = std::forward_iterator_tag;
            // To do
        };

        iterator begin() const;
        iterator end() const;
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string & word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string & noun1, std::string & noun2);

    // calculates distance between noun1 and noun2
    unsigned distance(const std::string & noun1, const std::string & noun2);
};
```
The input stream `synsets` has a CSV format with three columns:
* synset id
* synonyms
* gloss of the associated concept

![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-synsets.png)

The input stream `hypernyms` has a CSV format with a variable number of columns:
* the first column is the synset id
* the remaining columns are a list of its hypernyms

![](https://www.cs.princeton.edu/courses/archive/fall19/cos226/assignments/wordnet/images/wordnet-hypernyms.png)

To ensure the efficiency of the class implementation, we can set a limit on the length of the word string and the length of the concept string.

Requirements for the efficiency of the WordNet class:
* The class constructor should have a complexity of no worse than ```O(N + M)```, where N and M are the sizes of the input files
* The method `bool is_noun(const std::string & word) const` should have a complexity of no worse than ```O(log N)```, where N is the number of words
* The method `std::string sca(const std::string & noun1, std::string & noun2) const` should make exactly one call to `ShortestCommonAncestor::lengthSubset()`
* The method `unsigned distance(const std::string & noun1, const std::string & noun2) const` should make exactly one call to `ShortestCommonAncestor::ancestorSubset()`

### 2. Auxiliary class ShortestCommonAncestor for calculating the shortest common ancestor
```
class ShortestCommonAncestor
{
   ShortestCommonAncestor(const Digraph & G);

   // calculates length of shortest common ancestor path from node with id 'v' to node with id 'w'
   unsigned length(unsigned v, unsigned w);

   // returns node id of shortest common ancestor of nodes v and w
   unsigned ancestor(unsigned v, unsigned w);

   // calculates length of shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
   unsigned length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b);

   // returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
   unsigned ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b);
}
```
where Digraph is a class describing the graph ```G = (V, E)```.

Requirements for the efficiency of the ShortestCommonAncestor class: the constructor and methods of the class should be executed in time ```O(N_v + N_e)```, N_v is the size of the set of vertices V, N_e is the size of the set of edges E.

### 3. Outcast class for calculating the "extra" word from a sequence of words

```
class Outcast
{
public:
   explicit Outcast(WordNet & wordnet);

   // returns outcast word
   std::string outcast(const std::vector<std::string> & nouns);
};
```

### 4. Digraph class for describing the graph of concepts

```
class Digraph
{
...
};
```
The design of this class is part of the assignment. The obtained interface should meet the following requirements:
* The data of the class should be encapsulated.
* All suitable base operations (including output to stream operation) should be implemented.


## Note
* The proposed interfaces of the WordNet, ShortestCommonAncestor, and Outcast classes can be extended, but they cannot be changed. The introduction of additional user-defined data types is not limited.


## License

This project is licensed under the [GPL-3.0 license](LICENSE).
