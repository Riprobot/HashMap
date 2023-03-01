#pragma once

#include <typeindex>
#include <iterator>
#include <vector>
#include <memory>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    using ElementType = std::pair<const KeyType, ValueType>;

    struct ElementHash {
        std::unique_ptr<ElementType> element = nullptr;
        size_t hash = 0;

        ElementHash() {}

        ElementHash(ElementType element, size_t hash) : element(std::make_unique<ElementType>(element)), hash(hash) {}
    };

public:
    class iterator {
    public:
        iterator() : hm_(nullptr), index_(0) {}

        iterator(HashMap<KeyType, ValueType, Hash> *hm, size_t index) : hm_(hm), index_(index) {}

        ElementType &operator*() const {
            return *hm_->vec_[index_].element;
        }

        ElementType *operator->() const {
            return (hm_->vec_[index_].element).get();
        }

        iterator &operator=(iterator other) {
            if (this != &other) {
                hm_ = other.hm_;
                index_ = other.index_;
            }
            return *this;
        }

        iterator &operator++() {
            ++index_;
            index_ = hm_->next_bucket(index_);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(hm_, index_);
            ++index_;
            index_ = hm_->next_bucket(index_);
            return tmp;
        }

        bool operator==(const iterator &other) const {
            return hm_ == other.hm_ && index_ == other.index_;
        }

        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }

    private:
        HashMap<KeyType, ValueType, Hash> *hm_;
        size_t index_;
    };

    class const_iterator {
    public:
        const_iterator() : hm_(nullptr), index_(0) {}

        const_iterator(HashMap<KeyType, ValueType, Hash> const *hm, size_t index) : hm_(hm), index_(index) {}

        const ElementType &operator*() const {
            return *hm_->vec_[index_].element;
        }

        const ElementType *operator->() const {
            return (hm_->vec_[index_].element).get();
        }

        const_iterator &operator=(const_iterator other) {
            if (this != &other) {
                hm_ = other.hm_;
                index_ = other.index_;
            }
            return *this;
        }

        const_iterator &operator++() {
            ++index_;
            index_ = hm_->next_bucket(index_);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp(hm_, index_);
            ++index_;
            index_ = hm_->next_bucket(index_);
            return tmp;
        }

        bool operator==(const const_iterator &other) const {
            return hm_ == other.hm_ && index_ == other.index_;
        }

        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }

    private:
        HashMap<KeyType, ValueType, Hash> const *hm_;
        size_t index_;
    };

    HashMap(Hash hasher = Hash()) : hasher_(hasher) {
        vec_.resize(capacity_);
    }

    template<class Forwarditerator>
    HashMap(Forwarditerator begin, Forwarditerator end, Hash hasher = Hash()): hasher_(hasher) {
        vec_.resize(capacity_);
        while (begin != end) {
            insert(std::make_pair((*begin).first, (*begin).second));
            begin++;
        }
    }

    HashMap(std::initializer_list<ElementType> initializerList, Hash hasher = Hash()) {
        vec_.resize(capacity_);
        for (const auto &[key, value]: initializerList) {
            insert({key, value});
        }
    }

    HashMap(const HashMap &other) {
        hasher_ = other.hasher_;
        H = other.H;
        vec_.clear();
        capacity_ = other.capacity_;
        vec_.clear();
        vec_.resize(capacity_);
        auto begin_iterator = other.begin();
        auto end_iterator = other.end();
        while (begin_iterator != end_iterator) {
            insert(std::make_pair((*begin_iterator).first, (*begin_iterator).second));
            begin_iterator++;
        }
    }

    ~HashMap() {
        clear();
    }

    HashMap &operator=(const HashMap &other) {
        if (this == &other) {
            return *this;
        }
        clear();
        hasher_ = other.hasher_;
        H = other.H;
        capacity_ = other.capacity_;
        vec_.clear();
        vec_.resize(capacity_);
        auto begin_iterator = other.begin();
        auto end_iterator = other.end();
        while (begin_iterator != end_iterator) {
            insert(std::make_pair((*begin_iterator).first, (*begin_iterator).second));
            begin_iterator++;
        }
        return *this;
    }


    iterator begin() { // NOLINT
        return iterator(this, next_bucket(0));
    }

    iterator end() { // NOLINT
        return iterator(this, capacity_);
    }

    const_iterator begin() const { // NOLINT
        return const_iterator(this, next_bucket(0));
    }

    const_iterator end() const { // NOLINT
        return const_iterator(this, capacity_);
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }


    void insert(ElementType element) {
        if (find(element.first) != end()) {
            return;
        }
        ElementHash element_hash = ElementHash(element, hasher_(element.first) % capacity_);
        size_t begin = element_hash.hash;
        size_t pos = begin;
        while (vec_[pos].element != nullptr) {
            pos = sum(pos, 1);
        }
        while (distance(begin, pos) >= H) {
            bool found_swap = false;
            while (true) {
                size_t now = subtract(pos, 1);
                for (size_t i = 0; i < H - 1; ++i) {
                    if (distance(vec_[now].hash, pos) < H) {
                        found_swap = true;
                        std::swap(vec_[now], vec_[pos]);
                        pos = now;
                        break;
                    }
                    now = subtract(now, 1);
                }
                if (found_swap) {
                    break;
                }
                H *= 2;
            }
        }
        vec_[pos] = std::move(element_hash);
        size_ += 1;
        if (load_factor() >= max_load_factor) {
            rebuild();
        }
    }

    void erase(KeyType key) {
        size_t begin = hasher_(key) % capacity_;
        size_t pos = begin;
        for (size_t i = 0; i < H; ++i) {
            if (vec_[pos].element != nullptr && key == vec_[pos].element->first) {
                vec_[pos].element = nullptr;
                size_--;
                return;
            }
            pos = sum(pos, 1);
        }
    }

    iterator find(KeyType key) {
        size_t begin = hasher_(key) % capacity_;
        size_t pos = begin;
        for (size_t i = 0; i < H; ++i) {
            if (vec_[pos].element != nullptr && key == vec_[pos].element->first) {
                return iterator(this, pos);
            }
            pos = sum(pos, 1);
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t begin = hasher_(key) % capacity_;
        size_t pos = begin;
        for (size_t i = 0; i < H; ++i) {
            if (vec_[pos].element != nullptr && key == vec_[pos].element->first) {
                return const_iterator(this, pos);
            }
            pos = sum(pos, 1);
        }
        return end();
    }

    ValueType &operator[](KeyType key) {
        auto it = find(key);
        if (it == end()) {
            insert(std::make_pair(key, ValueType()));
            it = find(key);
        }
        return it->second;
    }

    const ValueType &at(KeyType key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("No ElementType at this key");
        }
        return it->second;
    }

    void clear() {
        size_ = 0;
        for (size_t i = 0; i < capacity_; ++i) {
            vec_[i].element = nullptr;
        }
    }


private:
    size_t next_bucket(size_t index) const {
        while (index != capacity_ && vec_[index].element == nullptr) {
            index++;
        }
        return index;
    }

    size_t sum(size_t a, size_t b) const {
        size_t c = a + b;
        if (c >= capacity_) {
            return c - capacity_;
        }
        return c;
    }

    size_t subtract(size_t a, size_t b) const {
        if (a >= b) {
            return a - b;
        } else {
            return a + capacity_ - b;
        }
    }

    size_t distance(size_t beg, size_t end) const {
        return subtract(end, beg);
    }

    double load_factor() const {
        return (double) size_ / capacity_;
    }

    void rebuild() {
        std::vector<ElementType> elements;
        for (auto element: *this) {
            elements.push_back(element);
        }
        clear();
        H = H_begin_value;
        capacity_ *= capacity_multiplier;
        vec_.resize(capacity_);
        for (auto element: elements) {
            insert(element);
        }
    }

    std::vector<ElementHash> vec_;
    Hash hasher_;
    size_t capacity_ = 64;
    size_t size_ = 0;
    const size_t H_begin_value = 2;
    size_t H = H_begin_value;
    const double capacity_multiplier = 2.0;
    const double max_load_factor = 0.90;
};
