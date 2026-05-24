#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <algorithm>
#include <concepts>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <limits>
#include <queue>
#include <stack>
#include <functional>

using i64 = int64_t;
using u64 = uint64_t;

using WeightT = u64;

// ─────────────────────────── forward declarations ───────────────────────────

template <std::equality_comparable LabelT, typename ValT> struct DEdge;
template <std::equality_comparable LabelT, typename ValT> struct Vertex;
template <std::equality_comparable LabelT, typename ValT> struct Graph;

template <std::equality_comparable LabelT, typename ValT>
using DEdgeContainer  = std::vector<DEdge<LabelT, ValT>>;

// VertexContainer — unordered_map: LabelT -> Vertex
template <std::equality_comparable LabelT, typename ValT>
using VertexContainer = std::unordered_map<LabelT, Vertex<LabelT, ValT>>;

// ─────────────────────────────── DEdge ──────────────────────────────────────

template <std::equality_comparable LabelT, typename ValT>
struct DEdge
{
    Vertex<LabelT, ValT>* from;
    Vertex<LabelT, ValT>* to;
    WeightT               weight;

    DEdge() = delete;
    DEdge(Vertex<LabelT, ValT>* from, Vertex<LabelT, ValT>* to, WeightT weight)
        : from(from), to(to), weight(weight) {}

    const LabelT& getFromLabel() const { return from->label; }
    const LabelT& getToLabel()   const { return to->label;   }
};

// ─────────────────────────────── Vertex ─────────────────────────────────────

template <std::equality_comparable LabelT, typename ValT>
struct Vertex
{
    friend struct Graph<LabelT, ValT>;

public:
    LabelT  label;
    WeightT weight;
    ValT    val;

private:
    DEdgeContainer<LabelT, ValT> inEdges;
    DEdgeContainer<LabelT, ValT> outEdges;

public:
    const auto& getInEdges()  const { return inEdges;  }
    const auto& getOutEdges() const { return outEdges; }

    template <typename V>
    requires std::convertible_to<V, ValT>
    Vertex(LabelT label, WeightT weight, V&& v)
        : label(std::move(label)), weight(weight), val(std::forward<V>(v)) {}

    Vertex(LabelT label, WeightT weight = 0)
        : label(std::move(label)), weight(weight), val{} {}

private:
    void addInEdge(Vertex<LabelT, ValT>* from, WeightT w)
    {
        inEdges.emplace_back(from, this, w);
    }
    void addOutEdge(Vertex<LabelT, ValT>* to, WeightT w)
    {
        outEdges.emplace_back(this, to, w);
    }

    void removeInEdge(Vertex<LabelT, ValT>* from)
    {
        auto it = std::find_if(inEdges.begin(), inEdges.end(),
            [from](const DEdge<LabelT, ValT>& e){ return e.from == from; });
        if (it != inEdges.end()) inEdges.erase(it);
    }
    void removeOutEdge(Vertex<LabelT, ValT>* to)
    {
        auto it = std::find_if(outEdges.begin(), outEdges.end(),
            [to](const DEdge<LabelT, ValT>& e){ return e.to == to; });
        if (it != outEdges.end()) outEdges.erase(it);
    }
};

// ─────────────────────────────── Graph ──────────────────────────────────────

template <std::equality_comparable LabelT = std::string, typename ValT = std::monostate>
struct Graph
{
public:
    class GraphException final : public std::exception
    {
        std::string message;
    public:
        GraphException(const std::string& errorMessage) : message(errorMessage) {}
        const char* what() const noexcept override { return message.c_str(); }
    };

public:
    VertexContainer<LabelT, ValT> vertices;

    // ── addVertex ────────────────────────────────────────────────────────────

    void addVertex(LabelT label, WeightT weight = 0)
    {
        auto [it, inserted] = vertices.emplace(label, Vertex<LabelT, ValT>(label, weight));
        if (!inserted)
            std::cout << "Node " << label << " already exists\n";
    }

    template <typename V>
    requires std::convertible_to<V, ValT>
    void addVertex(LabelT label, WeightT weight, V&& val)
    {
        auto [it, inserted] = vertices.emplace(label,
            Vertex<LabelT, ValT>(label, weight, std::forward<V>(val)));
        if (!inserted)
            std::cout << "Node " << label << " already exists\n";
    }

    // ── addEdge ──────────────────────────────────────────────────────────────

    bool addEdge(const LabelT& from, const LabelT& to, WeightT w)
    {
        auto* f = findVertex(from);
        auto* t = findVertex(to);

        if (!f && !t) { std::cout << "Unknown nodes " << from << " " << to << "\n"; return false; }
        if (!f)       { std::cout << "Unknown node "  << from << "\n";               return false; }
        if (!t)       { std::cout << "Unknown node "  << to   << "\n";               return false; }

        f->addOutEdge(t, w);
        t->addInEdge(f, w);
        return true;
    }

    // ── removeEdge ───────────────────────────────────────────────────────────

    bool removeEdge(const LabelT& from, const LabelT& to)
    {
        auto* f = findVertex(from);
        auto* t = findVertex(to);

        if (!f && !t) { std::cout << "Unknown nodes " << from << " " << to << "\n"; return false; }
        if (!f)       { std::cout << "Unknown node "  << from << "\n";               return false; }
        if (!t)       { std::cout << "Unknown node "  << to   << "\n";               return false; }

        f->removeOutEdge(t);
        t->removeInEdge(f);
        return true;
    }

    // ── removeVertex ─────────────────────────────────────────────────────────

    bool removeVertex(const LabelT& label)
    {
        auto* rm = findVertex(label);
        if (!rm) { std::cout << "Unknown node " << label << "\n"; return false; }

        for (auto& e : rm->outEdges) e.to->removeInEdge(rm);
        for (auto& e : rm->inEdges)  e.from->removeOutEdge(rm);

        vertices.erase(label);
        return true;
    }

    // ── RPO numbering ─────────────────────────────────────────────────────────

    void rpoNumbering(const LabelT& startLabel)
    {
        auto* start = findVertex(startLabel);
        if (!start) { std::cout << "Unknown node " << startLabel << "\n"; return; }

        std::unordered_map<const Vertex<LabelT, ValT>*, int> colour;
        std::vector<LabelT> postOrder;

        using EdgeIterator = typename DEdgeContainer<LabelT, ValT>::const_iterator;
        std::vector<std::pair<Vertex<LabelT, ValT>*, EdgeIterator>> stack;

        colour[start] = 1;
        stack.push_back({ start, start->getOutEdges().begin() });

        while (!stack.empty())
        {
            auto& [cur, edgeIt] = stack.back();

            if (edgeIt == cur->getOutEdges().end())
            {
                colour[cur] = 2;
                postOrder.push_back(cur->label);
                stack.pop_back();
            }
            else
            {
                auto* next = (*edgeIt).to;
                ++edgeIt;

                int c = colour.count(next) ? colour[next] : 0;
                if (c == 1)
                    std::cout << "Found loop " << cur->label << "->" << next->label << "\n";
                else if (c == 0)
                {
                    colour[next] = 1;
                    stack.push_back({ next, next->getOutEdges().begin() });
                }
            }
        }

        for (int i = (int)postOrder.size() - 1; i >= 0; --i)
        {
            if (i != (int)postOrder.size() - 1) std::cout << " ";
            std::cout << postOrder[i];
        }
        std::cout << "\n";
    }

    // ── Dijkstra ─────────────────────────────────────────────────────────────
    // Возвращает расстояния от startLabel до всех вершин.
    // Также выводит результат на stdout в формате: "<label> <dist>" по одному на строку,
    // отсортировано по возрастанию расстояния (недостижимые вершины не выводятся).

    std::unordered_map<LabelT, WeightT> dijkstra(const LabelT& startLabel)
    {
        auto* start = findVertex(startLabel);
        if (!start) throw GraphException("Unknown node " + startLabel);

        constexpr WeightT INF = std::numeric_limits<WeightT>::max();

        std::unordered_map<LabelT, WeightT> dist;
        for (const auto& [label, _] : vertices)
            dist[label] = INF;
        dist[startLabel] = 0;

        // min-heap: (distance, vertex pointer)
        // unordered_map не инвалидирует указатели при вставке новых элементов
        using PQItem = std::pair<WeightT, Vertex<LabelT, ValT>*>;
        std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;
        pq.push({ 0, start });

        while (!pq.empty())
        {
            auto [d, cur] = pq.top();
            pq.pop();

            // Устаревшая запись — пропускаем
            if (d > dist[cur->label]) continue;

            for (const auto& edge : cur->getOutEdges())
            {
                WeightT newDist = d + edge.weight;
                if (newDist < dist[edge.to->label])
                {
                    dist[edge.to->label] = newDist;
                    pq.push({ newDist, edge.to });
                }
            }
        }

        // Вывод: все вершины кроме стартовой, недостижимые пропускаются
        // Собираем в вектор для сортировки
        std::vector<std::pair<WeightT, LabelT>> results;
        for (const auto& [label, d] : dist)
        {
            if (label == startLabel) continue;
            if (d != INF)
                results.push_back({ d, label });
        }
        std::sort(results.begin(), results.end());

        for (const auto& [d, label] : results)
            std::cout << label << " " << d << "\n";

        return dist;
    }

    // ── Max Flow (Edmonds-Karp = Ford-Fulkerson + BFS) ────────────────────────
    // Синтаксис: MAX FLOW <source> <sink>
    // Выводит одно число — значение максимального потока.

    WeightT maxFlow(const LabelT& sourceLabel, const LabelT& sinkLabel)
    {
        auto* src = findVertex(sourceLabel);
        auto* snk = findVertex(sinkLabel);

        if (!src) throw GraphException("Unknown node " + sourceLabel);
        if (!snk) throw GraphException("Unknown node " + sinkLabel);

        // Остаточная сеть: capacity[u][v] — остаточная пропускная способность
        std::unordered_map<LabelT, std::unordered_map<LabelT, WeightT>> cap;

        // Инициализируем из рёбер графа
        for (const auto& [label, v] : vertices)
            for (const auto& e : v.getOutEdges())
                cap[e.getFromLabel()][e.getToLabel()] += e.weight;

        WeightT totalFlow = 0;

        // BFS для поиска увеличивающего пути
        auto bfs = [&](std::unordered_map<LabelT, LabelT>& parent) -> bool
        {
            std::unordered_map<LabelT, bool> visited;
            std::queue<LabelT> q;
            q.push(sourceLabel);
            visited[sourceLabel] = true;

            while (!q.empty())
            {
                LabelT cur = q.front(); q.pop();

                for (const auto& [next, residual] : cap[cur])
                {
                    if (!visited[next] && residual > 0)
                    {
                        visited[next] = true;
                        parent[next] = cur;
                        if (next == sinkLabel) return true;
                        q.push(next);
                    }
                }
            }
            return false;
        };

        std::unordered_map<LabelT, LabelT> parent;

        while (bfs(parent))
        {
            // Найти минимальную остаточную ёмкость вдоль пути
            WeightT pathFlow = std::numeric_limits<WeightT>::max();
            for (LabelT v = sinkLabel; v != sourceLabel; v = parent[v])
            {
                const LabelT& u = parent[v];
                pathFlow = std::min(pathFlow, cap[u][v]);
            }

            // Обновить остаточную сеть
            for (LabelT v = sinkLabel; v != sourceLabel; v = parent[v])
            {
                const LabelT& u = parent[v];
                cap[u][v] -= pathFlow;
                cap[v][u] += pathFlow;
            }

            totalFlow += pathFlow;
            parent.clear();
        }

        return totalFlow;
    }

    // ── Tarjan SCC ────────────────────────────────────────────────────────────
    // Находит все сильно связные компоненты (Strongly Connected Components).
    // Выводит компоненты с более чем 1 узлом: каждая на отдельной строке,
    // метки через пробел.

    void tarjan(const LabelT& startLabel)
    {
        // Проверка: стартовая вершина существует (по условию задачи передаётся метка)
        if (!findVertex(startLabel))
        {
            std::cout << "Unknown node " << startLabel << "\n";
            return;
        }

        std::unordered_map<LabelT, int>  index;   // порядок обхода
        std::unordered_map<LabelT, int>  lowlink; // минимальный index достижимый из поддерева
        std::unordered_map<LabelT, bool> onStack;
        std::stack<LabelT>               stk;

        int timer = 0;
        std::vector<std::vector<LabelT>> sccs;

        // Итеративная реализация алгоритма Тарьяна
        using EdgeIt = typename DEdgeContainer<LabelT, ValT>::const_iterator;
        struct Frame {
            Vertex<LabelT, ValT>* v;
            EdgeIt edgeIt;
        };

        std::function<void(Vertex<LabelT, ValT>*)> dfs = [&](Vertex<LabelT, ValT>* start_v)
        {
            std::vector<Frame> call_stack;
            call_stack.push_back({ start_v, start_v->getOutEdges().begin() });
            index[start_v->label]   = timer;
            lowlink[start_v->label] = timer;
            ++timer;
            stk.push(start_v->label);
            onStack[start_v->label] = true;

            while (!call_stack.empty())
            {
                auto& [cur, edgeIt] = call_stack.back();

                if (edgeIt == cur->getOutEdges().end())
                {
                    // Постобработка — обновляем lowlink родителя
                    if (call_stack.size() > 1)
                    {
                        auto& parent = call_stack[call_stack.size() - 2];
                        lowlink[parent.v->label] = std::min(
                            lowlink[parent.v->label],
                            lowlink[cur->label]);
                    }

                    // Корень SCC
                    if (lowlink[cur->label] == index[cur->label])
                    {
                        std::vector<LabelT> scc;
                        while (true)
                        {
                            LabelT top = stk.top(); stk.pop();
                            onStack[top] = false;
                            scc.push_back(top);
                            if (top == cur->label) break;
                        }
                        sccs.push_back(std::move(scc));
                    }
                    call_stack.pop_back();
                }
                else
                {
                    auto* next = edgeIt->to;
                    ++edgeIt;

                    if (index.find(next->label) == index.end())
                    {
                        // Не посещён — рекурсивно
                        index[next->label]   = timer;
                        lowlink[next->label] = timer;
                        ++timer;
                        stk.push(next->label);
                        onStack[next->label] = true;
                        call_stack.push_back({ next, next->getOutEdges().begin() });
                    }
                    else if (onStack[next->label])
                    {
                        // Back edge — обновляем lowlink
                        lowlink[cur->label] = std::min(lowlink[cur->label], index[next->label]);
                    }
                }
            }
        };

        // Запускаем обход из всех непосещённых вершин (граф может быть несвязным)
        for (auto& [label, v] : vertices)
        {
            if (index.find(label) == index.end())
                dfs(&v);
        }

        // Вывод компонент с более чем 1 узлом
        for (const auto& scc : sccs)
        {
            if (scc.size() > 1)
            {
                for (std::size_t i = 0; i < scc.size(); ++i)
                {
                    if (i) std::cout << " ";
                    std::cout << scc[i];
                }
                std::cout << "\n";
            }
        }
    }

private:
    Vertex<LabelT, ValT>* findVertex(const LabelT& label)
    {
        auto it = vertices.find(label);
        return (it == vertices.end()) ? nullptr : &(it->second);
    }
};

// ─────────────────────────── command-line driver ─────────────────────────────
// Принимает istream, что позволяет использовать как cin, так и файловый поток.

inline void runGraphDriver(std::istream& in = std::cin)
{
    Graph<> g;
    std::string line;

    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "NODE")
        {
            std::string label; ss >> label;
            g.addVertex(label);
        }
        else if (cmd == "EDGE")
        {
            std::string from, to; WeightT w = 0;
            ss >> from >> to >> w;
            g.addEdge(from, to, w);
        }
        else if (cmd == "REMOVE")
        {
            std::string sub; ss >> sub;
            if (sub == "NODE")
            {
                std::string label; ss >> label;
                g.removeVertex(label);
            }
            else if (sub == "EDGE")
            {
                std::string from, to;
                ss >> from >> to;
                g.removeEdge(from, to);
            }
            else
            {
                std::cout << "WRONG COMMAND\n";
            }
        }
        else if (cmd == "RPO_NUMBERING")
        {
            std::string label; ss >> label;
            g.rpoNumbering(label);
        }
        else if (cmd == "DIJKSTRA")
        {
            std::string label; ss >> label;
            try { g.dijkstra(label); }
            catch (const Graph<>::GraphException& e) { std::cout << e.what() << "\n"; }
        }
        else if (cmd == "MAX")
        {
            std::string sub; ss >> sub;
            if (sub == "FLOW")
            {
                std::string src, snk; ss >> src >> snk;
                try
                {
                    WeightT flow = g.maxFlow(src, snk);
                    std::cout << flow << "\n";
                }
                catch (const Graph<>::GraphException& e) { std::cout << e.what() << "\n"; }
            }
            else
            {
                std::cout << "WRONG COMMAND\n";
            }
        }
        else if (cmd == "TARJAN")
        {
            std::string label; ss >> label;
            g.tarjan(label);
        }
        else
        {
            std::cout << "WRONG COMMAND\n";
        }
    }
}