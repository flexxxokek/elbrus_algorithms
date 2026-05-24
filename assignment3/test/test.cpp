#include <graph.hpp>
#include <gtest/gtest.h>
#include <sstream>

// ─────────────────────────── helpers ─────────────────────────────────────────

// Запускает команды через драйвер и возвращает stdout как строку
static std::string run(const std::string& commands)
{
    std::istringstream in(commands);
    std::ostringstream out;
    auto* oldBuf = std::cout.rdbuf(out.rdbuf());
    runGraphDriver(in);
    std::cout.rdbuf(oldBuf);
    return out.str();
}

// ─────────────────────────── NODE / EDGE / REMOVE ────────────────────────────

TEST(GraphManagement, AddAndRemoveVertex)
{
    Graph<> g;
    g.addVertex("A");
    g.addVertex("B");
    EXPECT_EQ(g.vertices.size(), 2u);

    g.removeVertex("A");
    EXPECT_EQ(g.vertices.size(), 1u);
    EXPECT_EQ(g.vertices.count("A"), 0u);
}

TEST(GraphManagement, DuplicateVertex)
{
    std::string out = run("NODE A\nNODE A\n");
    EXPECT_NE(out.find("already exists"), std::string::npos);
}

TEST(GraphManagement, AddEdgeUnknownNodes)
{
    EXPECT_NE(run("EDGE A B 5\n").find("Unknown nodes A B"), std::string::npos);
    EXPECT_NE(run("NODE A\nEDGE A B 5\n").find("Unknown node B"),  std::string::npos);
    EXPECT_NE(run("NODE B\nEDGE A B 5\n").find("Unknown node A"),  std::string::npos);
}

TEST(GraphManagement, RemoveEdgeCleansAdjacency)
{
    Graph<> g;
    g.addVertex("A");
    g.addVertex("B");
    g.addEdge("A", "B", 7);

    ASSERT_EQ(g.vertices.at("A").getOutEdges().size(), 1u);
    ASSERT_EQ(g.vertices.at("B").getInEdges().size(),  1u);

    g.removeEdge("A", "B");

    EXPECT_TRUE(g.vertices.at("A").getOutEdges().empty());
    EXPECT_TRUE(g.vertices.at("B").getInEdges().empty());
}

TEST(GraphManagement, RemoveVertexCleansNeighborEdges)
{
    Graph<> g;
    g.addVertex("A");
    g.addVertex("B");
    g.addVertex("C");
    g.addEdge("A", "B", 1);
    g.addEdge("B", "C", 1);

    g.removeVertex("B");

    EXPECT_TRUE(g.vertices.at("A").getOutEdges().empty());
    EXPECT_TRUE(g.vertices.at("C").getInEdges().empty());
}

// ─────────────────────────── RPO_NUMBERING ───────────────────────────────────

TEST(RpoTests, SimpleCases)
{
    // Линейная цепочка A->B->C: RPO = A B C
    std::string out = run(
        "NODE A\nNODE B\nNODE C\n"
        "EDGE A B 1\nEDGE B C 1\n"
        "RPO_NUMBERING A\n"
    );
    EXPECT_EQ(out, "A B C\n");
}

TEST(RpoTests, DetectsLoop)
{
    std::string out = run(
        "NODE A\nNODE B\nNODE C\n"
        "EDGE A B 1\nEDGE B C 1\nEDGE C A 1\n"
        "RPO_NUMBERING A\n"
    );
    EXPECT_NE(out.find("Found loop"), std::string::npos);
}

TEST(RpoTests, UnknownStartNode)
{
    std::string out = run("RPO_NUMBERING Z\n");
    EXPECT_NE(out.find("Unknown node Z"), std::string::npos);
}

TEST(RpoTests, DiamondGraph)
{
    //   A
    //  / \
    // B   C
    //  \ /
    //   D
    // Допустимые RPO: A B C D  или  A C B D
    std::string out = run(
        "NODE A\nNODE B\nNODE C\nNODE D\n"
        "EDGE A B 1\nEDGE A C 1\nEDGE B D 1\nEDGE C D 1\n"
        "RPO_NUMBERING A\n"
    );
    EXPECT_EQ(out.substr(0, 1), "A");
    EXPECT_NE(out.find("D"), std::string::npos);
}

// ─────────────────────────── DIJKSTRA ────────────────────────────────────────

TEST(DijkstraTests, BasicShortestPaths)
{
    // A --10--> B
    // A --5-->  C
    // C --3-->  B   => dist(B) = 8
    // B --1-->  D   => dist(D) = 9
    std::string out = run(
        "NODE A\nNODE B\nNODE C\nNODE D\n"
        "EDGE A B 10\nEDGE A C 5\nEDGE C B 3\nEDGE B D 1\n"
        "DIJKSTRA A\n"
    );
    EXPECT_NE(out.find("C 5"), std::string::npos);
    EXPECT_NE(out.find("B 8"), std::string::npos);
    EXPECT_NE(out.find("D 9"), std::string::npos);
}

TEST(DijkstraTests, UnreachableNodeNotPrinted)
{
    // D не достижима из A
    std::string out = run(
        "NODE A\nNODE B\nNODE D\n"
        "EDGE A B 3\n"
        "DIJKSTRA A\n"
    );
    EXPECT_NE(out.find("B 3"), std::string::npos);
    EXPECT_EQ(out.find(" D "), std::string::npos); // D не должна выводиться
}

TEST(DijkstraTests, SingleVertex)
{
    // Только стартовая вершина — вывод пустой
    std::string out = run("NODE A\nDIJKSTRA A\n");
    EXPECT_EQ(out, "");
}

TEST(DijkstraTests, UnknownStartNode)
{
    std::string out = run("NODE A\nDIJKSTRA Z\n");
    EXPECT_NE(out.find("Unknown node Z"), std::string::npos);
}

TEST(DijkstraTests, LinearChain)
{
    // A -1-> B -1-> C -1-> D  => дистанции 1, 2, 3
    std::string out = run(
        "NODE A\nNODE B\nNODE C\nNODE D\n"
        "EDGE A B 1\nEDGE B C 1\nEDGE C D 1\n"
        "DIJKSTRA A\n"
    );
    EXPECT_NE(out.find("B 1"), std::string::npos);
    EXPECT_NE(out.find("C 2"), std::string::npos);
    EXPECT_NE(out.find("D 3"), std::string::npos);
}

// ─────────────────────────── MAX FLOW ────────────────────────────────────────

TEST(MaxFlowTests, SimpleParallelPaths)
{
    // S->A->T и S->B->T, каждый путь по 10 => поток 20
    std::string out = run(
        "NODE S\nNODE A\nNODE B\nNODE T\n"
        "EDGE S A 10\nEDGE S B 10\nEDGE A T 10\nEDGE B T 10\n"
        "MAX FLOW S T\n"
    );
    EXPECT_NE(out.find("20"), std::string::npos);
}

TEST(MaxFlowTests, BottleneckEdge)
{
    // S->A (100), A->T (1) => узкое место = 1
    std::string out = run(
        "NODE S\nNODE A\nNODE T\n"
        "EDGE S A 100\nEDGE A T 1\n"
        "MAX FLOW S T\n"
    );
    EXPECT_NE(out.find("1"), std::string::npos);
}

TEST(MaxFlowTests, SameSourceAndSink)
{
    // Исток == сток => поток = 0
    std::string out = run(
        "NODE A\nNODE B\nEDGE A B 5\n"
        "MAX FLOW A A\n"
    );
    EXPECT_NE(out.find("0"), std::string::npos);
}

TEST(MaxFlowTests, UnknownNode)
{
    std::string out = run("NODE A\nMAX FLOW A Z\n");
    EXPECT_NE(out.find("Unknown node Z"), std::string::npos);
}

TEST(MaxFlowTests, ClassicFordFulkerson)
{
    // Классический пример из учебников:
    //        10       10
    //   S -------> A -------> T
    //   |    10    |    1     ^
    //   +-------> B -------->+
    // Ожидаемый максимальный поток = 20
    std::string out = run(
        "NODE S\nNODE A\nNODE B\nNODE T\n"
        "EDGE S A 10\nEDGE S B 10\n"
        "EDGE A T 10\nEDGE B T 10\nEDGE A B 1\n"
        "MAX FLOW S T\n"
    );
    EXPECT_NE(out.find("20"), std::string::npos);
}

// ─────────────────────────── TARJAN ──────────────────────────────────────────

TEST(TarjanTests, SingleSCC)
{
    // A->B->C->A — одна SCC из 3 узлов
    std::string out = run(
        "NODE A\nNODE B\nNODE C\n"
        "EDGE A B 1\nEDGE B C 1\nEDGE C A 1\n"
        "TARJAN A\n"
    );
    EXPECT_NE(out.find("A"), std::string::npos);
    EXPECT_NE(out.find("B"), std::string::npos);
    EXPECT_NE(out.find("C"), std::string::npos);
}

TEST(TarjanTests, NoNonTrivialSCC)
{
    // Линейная цепочка — нет SCC с более чем 1 узлом, вывод пустой
    std::string out = run(
        "NODE A\nNODE B\nNODE C\n"
        "EDGE A B 1\nEDGE B C 1\n"
        "TARJAN A\n"
    );
    EXPECT_EQ(out, "");
}

TEST(TarjanTests, TwoSeparateSCCs)
{
    // A<->B и C<->D — две SCC, каждая по 2 узла
    std::string out = run(
        "NODE A\nNODE B\nNODE C\nNODE D\n"
        "EDGE A B 1\nEDGE B A 1\n"
        "EDGE C D 1\nEDGE D C 1\n"
        "TARJAN A\n"
    );
    // Обе компоненты должны быть в выводе
    EXPECT_NE(out.find("A"), std::string::npos);
    EXPECT_NE(out.find("B"), std::string::npos);
    EXPECT_NE(out.find("C"), std::string::npos);
    EXPECT_NE(out.find("D"), std::string::npos);
    // Две строки
    long lines = std::count(out.begin(), out.end(), '\n');
    EXPECT_EQ(lines, 2);
}

TEST(TarjanTests, UnknownStartNode)
{
    std::string out = run("TARJAN Z\n");
    EXPECT_NE(out.find("Unknown node Z"), std::string::npos);
}

TEST(TarjanTests, MixedGraph)
{
    // A->B->C->B (B-C — SCC), A — отдельная вершина
    std::string out = run(
        "NODE A\nNODE B\nNODE C\n"
        "EDGE A B 1\nEDGE B C 1\nEDGE C B 1\n"
        "TARJAN A\n"
    );
    EXPECT_NE(out.find("B"), std::string::npos);
    EXPECT_NE(out.find("C"), std::string::npos);
    // A не должна быть в SCC (она одна)
    // Ровно одна строка вывода
    long lines = std::count(out.begin(), out.end(), '\n');
    EXPECT_EQ(lines, 1);
}