#ifndef WORDNET_WORDNET_HPP
#define WORDNET_WORDNET_HPP
#include <cstddef>
#include <deque>
#include <iosfwd>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Digraph {
public:
    Digraph();
    void extend(std::size_t size);
    void add_edge(unsigned v, unsigned w);
    std::size_t size() const;
    // Please implement...
private:
    friend class ShortestCommonAncestor;
    std::vector<std::vector<unsigned>> graph;
    std::unordered_map<unsigned, unsigned> id_map;
    std::vector<unsigned> node_map;
    unsigned build(unsigned v);
};

class ShortestCommonAncestor {
    friend class WordNet;
    const Digraph& digraph;
    explicit ShortestCommonAncestor(const Digraph&);
    [[nodiscard]] std::pair<unsigned, unsigned> bfs(const std::vector<unsigned>& subset_a,
                                                    const std::vector<unsigned>& subset_b);
    // calculates length of shortest common ancestor path from node with id 'v' to
    // node with id 'w'
    unsigned length(unsigned v, unsigned w);

    // returns node id of shortest common ancestor of nodes v and w
    unsigned ancestor(unsigned v, unsigned w);
    // calculates length of shortest common ancestor path from node subset
    // 'subset_a' to node subset 'subset_b'
    unsigned length_subset(const std::set<unsigned>& subset_a, const std::set<unsigned>& subset_b);

    // returns node id of shortest common ancestor of node subset 'subset_a' and
    // node subset 'subset_b'
    unsigned ancestor_subset(const std::set<unsigned>& subset_a, const std::set<unsigned>& subset_b);
};

class WordNet {
    using data_type = std::unordered_map<std::string, std::vector<unsigned>>;
    std::vector<std::string> input;
    std::unordered_map<unsigned, std::string> glosses;
    data_type words;
    Digraph digraph;
    std::pair<unsigned, unsigned> distance_calculation(const std::string& noun1, const std::string& noun2) const;

public:
    WordNet(std::istream& synsets, std::istream& hypernyms);

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
    class Nouns {
    private:
        explicit Nouns(const data_type&);
        friend class WordNet;
        const data_type& wordnet;

    public:
        class iterator {
        public:
            using difference_type   = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;
            using value_type        = std::string;
            using pointer           = const value_type*;
            using reference         = const value_type&;
            iterator();

            bool operator==(const iterator& another) const;

            bool operator!=(const iterator& another) const;

            reference operator*() const;

            pointer operator->() const;

            iterator& operator++();

            iterator operator++(int);

        private:
            friend class Nouns;

            explicit iterator(data_type::const_iterator);

            data_type::const_iterator it;
        };

        iterator begin() const;
        iterator end() const;
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string& word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string& noun1, const std::string& noun2) const;

    // calculates distance between noun1 and noun2
    unsigned distance(const std::string& noun1, const std::string& noun2) const;
};

class Outcast {
    WordNet& wordNet;

public:
    explicit Outcast(WordNet&);

    // returns outcast word
    std::string outcast(const std::set<std::string>& nouns);
};

#endif  // WORDNET_WORDNET_HPP
