#include "wordnet/Wordnet.hpp"

#include <charconv>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <vector>

Digraph::Digraph() = default;

std::size_t Digraph::size() const {
    return graph.size();
}

unsigned string_view_to_unsigned(std::string_view sv) {
    unsigned result;
    std::from_chars(sv.begin(), sv.end(), result);
    return result;
}

void Digraph::extend(std::size_t new_value) {
    graph.reserve(new_value);
    node_map.reserve(new_value);
    id_map.reserve(new_value);
}

void Digraph::add_edge(unsigned v, unsigned w) {
    graph[build(v)].emplace_back(build(w));
}

unsigned Digraph::build(unsigned id) {
    auto it = id_map.find(id);
    if (it == id_map.end()) {
        it = id_map.emplace(id, node_map.size()).first;
        node_map.emplace_back(id);
        graph.emplace_back();
    }
    return it->second;
}

std::pair<unsigned, unsigned> ShortestCommonAncestor::bfs(const std::vector<unsigned>& subset_a,
                                                          const std::vector<unsigned>& subset_b) {
    std::vector<unsigned> distance(digraph.size());
    std::vector<unsigned> marked(digraph.size());
    std::deque<unsigned> queue;

    for (auto it : subset_a) {
        unsigned node = digraph.id_map.at(it);
        marked[node]  = 1;
        queue.emplace_back(node);
    }
    for (auto it : subset_b) {
        unsigned node = digraph.id_map.at(it);
        if (marked[node])
            return std::make_pair(it, 0);
        marked[node] = 2;
        queue.emplace_back(node);
    }
    unsigned min_distance = 4294967290;
    unsigned ancestor_id  = 0;
    for (; !queue.empty(); queue.pop_front()) {
        const unsigned node = queue.front();
        for (auto it : digraph.graph[node]) {
            if (!marked[it]) {
                marked[it]   = marked[node];
                distance[it] = distance[node] + 1;
                queue.emplace_back(it);
            } else if (marked[it] != marked[node] && distance[it] + distance[node] + 1 < min_distance) {
                min_distance = distance[it] + distance[node] + 1;
                ancestor_id  = it;
            }
        }
    }
    return std::make_pair(digraph.node_map[ancestor_id], min_distance);
}
ShortestCommonAncestor::ShortestCommonAncestor(const Digraph& dg) : digraph(dg) {}

unsigned ShortestCommonAncestor::length(unsigned int v, unsigned int w) {
    return bfs({v}, {w}).second;
}
unsigned ShortestCommonAncestor::ancestor(unsigned int v, unsigned int w) {
    return bfs({v}, {w}).first;
}
unsigned ShortestCommonAncestor::length_subset(const std::set<unsigned int>& subset_a,
                                               const std::set<unsigned int>& subset_b) {
    return bfs({subset_a.begin(), subset_a.end()}, {subset_b.begin(), subset_b.end()}).second;
}
unsigned ShortestCommonAncestor::ancestor_subset(const std::set<unsigned int>& subset_a,
                                                 const std::set<unsigned int>& subset_b) {
    return bfs({subset_a.begin(), subset_a.end()}, {subset_b.begin(), subset_b.end()}).first;
}

WordNet::WordNet(std::istream& synsets, std::istream& hypernyms) {
    unsigned unsigned_string;
    std::size_t gloss_start;
    std::size_t start_pos;
    std::size_t end_pos;

    std::string line;
    while (std::getline(synsets, line)) {
        if (line.empty()) {
            continue;
        }
        input.emplace_back(std::move(line));
    }

    for (auto& it : input) {
        std::string identifier(&it[0], it.find(','));
        unsigned_string = string_view_to_unsigned(identifier);
        gloss_start     = it.find(',', identifier.size() + 1) + 1;
        start_pos       = identifier.size() + 1;
        while ((end_pos = it.find(' ', start_pos)) < gloss_start) {
            words[{&it[start_pos], end_pos - start_pos}].emplace_back(unsigned_string);
            start_pos = end_pos + 1;
        }
        words[{&it[start_pos], gloss_start - 1 - start_pos}].emplace_back(unsigned_string);
        glosses[unsigned_string] = {&it[gloss_start], it.size() - gloss_start};
    }

    digraph.extend(input.size());
    while (std::getline(hypernyms, line)) {
        if (line.empty()) {
            continue;
        }
        std::string_view identifier(&line[0], line.find(','));
        if (identifier.size() < line.size()) {
            unsigned_string = string_view_to_unsigned(identifier);
            start_pos       = identifier.size() + 1;
            while ((end_pos = line.find(',', start_pos)) < line.size()) {
                digraph.add_edge(unsigned_string, string_view_to_unsigned({&line[start_pos], end_pos - start_pos}));
                start_pos = end_pos + 1;
            }
            digraph.add_edge(unsigned_string, string_view_to_unsigned({&line[start_pos], line.size() - start_pos}));
        }
    }
}

std::pair<unsigned, unsigned> WordNet::distance_calculation(const std::string& noun1, const std::string& noun2) const {
    return ShortestCommonAncestor(digraph).bfs(words.at(noun1), words.at(noun2));
}

WordNet::Nouns::iterator::iterator() = default;
bool WordNet::Nouns::iterator::operator==(const WordNet::Nouns::iterator& another) const {
    return it == another.it;
}
bool WordNet::Nouns::iterator::operator!=(const WordNet::Nouns::iterator& another) const {
    return !(*this == another);
}
const WordNet::Nouns::iterator::value_type& WordNet::Nouns::iterator::operator*() const {
    return (*it).first;
}
WordNet::Nouns::iterator::pointer WordNet::Nouns::iterator::operator->() const {
    return &it->first;
}
WordNet::Nouns::iterator& WordNet::Nouns::iterator::operator++() {
    it++;
    return *this;
}
WordNet::Nouns::iterator WordNet::Nouns::iterator::operator++(int) {
    auto tmp = *this;
    operator++();
    return tmp;
}
WordNet::Nouns::iterator::iterator(data_type::const_iterator _it) : it(_it) {}

WordNet::Nouns::iterator WordNet::Nouns::begin() const {
    return iterator(wordnet.begin());
}
WordNet::Nouns::iterator WordNet::Nouns::end() const {
    return iterator(wordnet.end());
}
WordNet::Nouns::Nouns(const data_type& _wordnet) : wordnet(_wordnet) {}

WordNet::Nouns WordNet::nouns() const {
    return Nouns(words);
}
bool WordNet::is_noun(const std::string& word) const {
    return words.find(word) != words.end();
}

std::string WordNet::sca(const std::string& noun1, const std::string& noun2) const {
    return std::string(glosses.at(distance_calculation(noun1, noun2).first));
}

unsigned WordNet::distance(const std::string& noun1, const std::string& noun2) const {
    return distance_calculation(noun1, noun2).second;
}

Outcast::Outcast(WordNet& _wordNet) : wordNet(_wordNet) {}

std::string Outcast::outcast(const std::set<std::string>& nouns) {
    if (nouns.size() <= 2)
        return "";
    std::vector<unsigned> distances(nouns.size());
    std::size_t ans = 0;
    std::set<std::string>::iterator it;
    bool confirmed             = false;
    std::size_t own_iterator_1 = 0;
    std::size_t onw_iterator_2;
    for (auto first_nout = nouns.begin(); first_nout != nouns.end(); first_nout++, ++own_iterator_1) {
        onw_iterator_2 = own_iterator_1 + 1;
        for (auto second_nout = std::next(first_nout); second_nout != nouns.end(); ++second_nout, ++onw_iterator_2) {
            unsigned distance = wordNet.distance(*first_nout, *second_nout);
            distances[own_iterator_1] += distance;
            distances[onw_iterator_2] += distance;
        }
        if (distances[own_iterator_1] > distances[ans] || !own_iterator_1) {
            ans       = own_iterator_1;
            it        = first_nout;
            confirmed = false;
            continue;
        }
        confirmed = distances[own_iterator_1] == distances[ans] || confirmed;
    }
    return (confirmed ? "" : *it);
}
