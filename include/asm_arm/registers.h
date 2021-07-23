#ifndef TINBACCC_REGISTERS_H
#define TINBACCC_REGISTERS_H

#include <set>
#include <vector>
#include <unordered_map>
#include <utility>
#include <asm_arm/instructions.h>
#include <stack>
#include <algorithm>


namespace asm_arm {
    const int K = 12;
    typedef std::pair<Operand *, Operand *> Edge;
    typedef std::vector<Operand *> OperandList;
    typedef std::set<Operand *> OperandSet;
    typedef std::set<Edge> EdgeSet;
    typedef std::stack<Operand *> OperandStack;
    typedef std::set<int> ColorSet;

    typedef std::set<Inst *> InstSet;
    typedef std::vector<Inst *> InstList;
    typedef std::set<MOVInst *> MOVInstSet;
    typedef std::vector<MOVInst *> MOVInstList;

    typedef std::unordered_map<Operand *, OperandSet> Operand2OperandSet;
    typedef std::unordered_map<Operand *, OperandList> Operand2OperandList;
    typedef std::unordered_map<Operand *, InstSet> Operand2InstSet;
    typedef std::unordered_map<Operand *, InstList> Operand2InstList;
    typedef std::unordered_map<Operand *, Operand *> Operand2Operand;

    typedef std::unordered_map<Operand *, int> Operand2Int;


    class RegisterAllocator {
    private:
        // Node
        OperandSet  preColored;         // machine registers, preassigned a color.
        OperandSet  initial;            // machine registers, preassigned a color.
        OperandList simplifyWorklist;   // list of low-degree non-move-related nodes.
        OperandSet  freezeWorklist;     // low-degree move-related nodes.
        OperandSet  spillWorklist;      // high-degree nodes.
        OperandList spilledNodes;       // nodes marked for spilling during this round; initially empty.
        OperandSet  coalescedNodes;     // registers that have been coalesced.
        OperandList coloredNodes;       // nodes successfully colored.
        OperandList selectStack;        // stack containing temporaries removed from the graph.

        // Move Sets: for move instructions
        MOVInstSet coalescedMoves;      // moves that have been coalesced.
        MOVInstSet constrainedMoves;    // moves whose source and target interfere.
        MOVInstSet frozenMoves;         // moves that will no longer be considered for coalescing.
        MOVInstSet worklistMoves;       // moves enabled for possible coalescing.
        MOVInstSet activeMoves;         // moves not yet ready for coalescing.


        // Graph
        EdgeSet             adjSet;     // the symmetrical set of interference edges (u, v) in the graph.
        Operand2OperandList adjList;    // We represent adjSet as a hash table of integer pairs.
        Operand2Int         degree;     // an array containing the current degree of each node.
        Operand2InstSet     moveList;   // a mapping from node to the list of moves it is associated with.
        Operand2Operand     alias;      // (u, v) has been coalesced, and v put in coalescedNodes, then alias(v) = u.
        Operand2Int         color;      // the color chosen by the algorithm for a node.


        asm_arm::Module *module;
        asm_arm::Function function;


        void livenessAnalysis();

        void build();

        void mkWorklist();

        void insertAdjList(Operand *u, Operand *v);

        void addEdge(Operand *u, Operand *v);

        void simplify();

        void coalesce();

        void freeze();

        void selectSpill();

        void assignColors();

        OperandList adjacent(Operand *n) const;

        MOVInstSet nodeMoves(Operand *n);

        bool isMoveRelated(Operand *n) const;

        bool isOK(Operand *t, Operand *r) const;


        void decrementDegree(Operand *m);

        bool isConservative(OperandList &nodes) const;

        Operand *getAlias(Operand *n);

        void addWorkList(Operand *u);

        void enableMoves(OperandList &nodes);

        void combine(Operand *u, Operand *v);

        void freezeMoves(Operand *u);

        void rewriteProgram(OperandList nodes);

    public:

        void allocatedRegister();

    };
}
#endif //TINBACCC_REGISTERS_H
