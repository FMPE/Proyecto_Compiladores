#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

template <typename T>
class Environment {
private:
    std::vector<std::unordered_map<std::string, T>> scopes;

    int find_scope_index(const std::string& name) const {
        for (int idx = static_cast<int>(scopes.size()) - 1; idx >= 0; --idx) {
            const auto& scope = scopes[idx];
            if (scope.find(name) != scope.end()) {
                return idx;
            }
        }
        return -1;
    }

public:
    Environment() = default;

    void clear() {
        scopes.clear();
    }

    void push_scope() {
        scopes.emplace_back();
    }

    void pop_scope() {
        if (scopes.empty()) {
            throw std::runtime_error("Environment::pop_scope sin scopes disponibles");
        }
        scopes.pop_back();
    }

    std::size_t depth() const {
        return scopes.size();
    }

    bool empty() const {
        return scopes.empty();
    }

    bool declare(const std::string& name, const T& value) {
        if (scopes.empty()) {
            push_scope();
        }
        scopes.back()[name] = value;
        return true;
    }

    bool assign(const std::string& name, const T& value) {
        int scopeIndex = find_scope_index(name);
        if (scopeIndex < 0) {
            return false;
        }
        scopes[static_cast<std::size_t>(scopeIndex)][name] = value;
        return true;
    }

    bool contains(const std::string& name) const {
        return find_scope_index(name) >= 0;
    }

    bool contains_current_scope(const std::string& name) const {
        if (scopes.empty()) {
            return false;
        }
        return scopes.back().find(name) != scopes.back().end();
    }

    T* lookup(const std::string& name) {
        int scopeIndex = find_scope_index(name);
        if (scopeIndex < 0) {
            return nullptr;
        }
        auto& scope = scopes[static_cast<std::size_t>(scopeIndex)];
        typename std::unordered_map<std::string, T>::iterator it = scope.find(name);
        return (it != scope.end()) ? &it->second : nullptr;
    }

    const T* lookup(const std::string& name) const {
        int scopeIndex = find_scope_index(name);
        if (scopeIndex < 0) {
            return nullptr;
        }
        const auto& scope = scopes[static_cast<std::size_t>(scopeIndex)];
        typename std::unordered_map<std::string, T>::const_iterator it = scope.find(name);
        return (it != scope.end()) ? &it->second : nullptr;
    }
};

#endif // ENVIRONMENT_H
