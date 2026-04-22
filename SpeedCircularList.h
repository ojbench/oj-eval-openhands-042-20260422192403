#ifndef SPEEDCIRCULARLIST_H
#define SPEEDCIRCULARLIST_H
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

constexpr int s_prime = 31;

inline int log2(int x) {
    int ans = 0;
    while (x > 1) {
        x /= 2;
        ++ans;
    }
    return ans;
}

template<typename T, int b_prime>
class SpeedCircularLinkedList {
public:
    struct Node {
        std::map<std::string, T> kv_map;
        int bound = 0;
        Node* next = nullptr;
        Node** fast_search_list = nullptr;
        int idx = -1; // position in ring starting from head as 0

        Node(int b, int fast_size) : bound(b) {
            if (fast_size > 0) {
                fast_search_list = new Node*[fast_size];
                for (int i = 0; i < fast_size; ++i) fast_search_list[i] = nullptr;
            } else {
                fast_search_list = nullptr;
            }
        }

        ~Node() {
            if (fast_search_list) {
                delete[] fast_search_list;
                fast_search_list = nullptr;
            }
            next = nullptr;
        }
    };

private:
    Node* head = nullptr;
    int fast_search_list_size = 0;
    int list_size = 0;

    static int GetHashCode(std::string str) {
        long long ans = 0;
        for (auto& ch : str) {
            ans = (ans * s_prime + ch) % b_prime;
        }
        return static_cast<int>((ans + b_prime) % b_prime);
    }

    void BuildFastSearchList() {
        if (!head || list_size <= 1 || fast_search_list_size <= 0) return;
        std::vector<Node*> nodes;
        nodes.reserve(list_size);
        Node* cur = head;
        for (int i = 0; i < list_size; ++i) {
            nodes.push_back(cur);
            cur = cur->next;
        }
        // Initialize level 0 for all nodes first
        for (auto* node : nodes) {
            node->fast_search_list[0] = node->next;
        }
        // Build higher levels using previously built levels
        for (int k = 1; k < fast_search_list_size; ++k) {
            for (auto* node : nodes) {
                Node* step = node->fast_search_list[k-1];
                node->fast_search_list[k] = step->fast_search_list[k-1];
            }
        }
    }

public:
    explicit SpeedCircularLinkedList(std::vector<int> node_bounds) {
        list_size = static_cast<int>(node_bounds.size());
        fast_search_list_size = (list_size <= 1) ? 0 : log2(list_size);
        if (list_size == 0) {
            head = nullptr;
            return;
        }
        Node* prev = nullptr;
        for (int i = 0; i < list_size; ++i) {
            Node* node = new Node(node_bounds[i], fast_search_list_size);
            if (!head) head = node;
            if (prev) prev->next = node;
            prev = node;
        }
        if (prev) prev->next = head;
        BuildFastSearchList();
    }

    ~SpeedCircularLinkedList() {
        if (!head || list_size == 0) return;
        Node* cur = head;
        for (int i = 0; i < list_size; ++i) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head = nullptr;
        list_size = 0;
        fast_search_list_size = 0;
    }

    Node* find_target_by_code(int code) {
        if (!head) return nullptr;
        if (code <= head->bound) return head;
        if (fast_search_list_size <= 0) {
            Node* cur = head->next;
            while (code > cur->bound) cur = cur->next;
            return cur;
        }
        Node* cur = head;
        for (int k = fast_search_list_size - 1; k >= 0; --k) {
            Node* nxt = cur->fast_search_list[k];
            if (!nxt) continue;
            if (nxt == head) continue; // do not wrap
            if (nxt->bound > cur->bound && nxt->bound < code) cur = nxt;
        }
        while (cur->next != head && cur->next->bound < code) cur = cur->next;
        return cur->next; // first >= code
    }

    void put(std::string str, T value) {
        int code = GetHashCode(str);
        if (!head) return;
        Node* target = find_target_by_code(code);
        target->kv_map[str] = value;
    }

    T get(std::string str) {
        int code = GetHashCode(str);
        if (!head) return T();
        Node* target = find_target_by_code(code);
        auto it = target->kv_map.find(str);
        if (it == target->kv_map.end()) return T();
        return it->second;
    }

    void print() {
        Node* cur = head;
        for (int i = 0; i < list_size; ++i) {
            std::cout << "[Node] Bound = " << cur->bound << ", kv_map_size = " << cur->kv_map.size() << "\n";
            cur = cur->next;
        }
    }

    int size() const { return list_size; }
};

#endif // SPEEDCIRCULARLIST_H
