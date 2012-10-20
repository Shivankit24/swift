//===--- CFGGen.h - Implements Lowering of ASTs -> CFGs ---------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef CFGGEN_H
#define CFGGEN_H

#include "Cleanup.h"
#include "swift/CFG/CFG.h"
#include "swift/CFG/CFGBuilder.h"
#include "swift/AST/ASTVisitor.h"
#include "llvm/ADT/DenseMap.h"

namespace swift {
  class BasicBlock;
  
namespace Lowering {
  class Condition;

class LLVM_LIBRARY_VISIBILITY CFGGen
  : public ASTVisitor<CFGGen, Value*, void> {
public:
  /// The CFG being constructed.
  CFG &C;
  
  /// B - The CFGBuilder used to construct the CFG.  It is what maintains the
  /// notion of the current block being emitted into.
  CFGBuilder B;
  
  std::vector<JumpDest> BreakDestStack;
  std::vector<JumpDest> ContinueDestStack;

  /// Cleanups - This records information about the currently active cleanups.
  CleanupManager Cleanups;

  /// VarLocs - This is the memory location that an emitted variable is stored.
  /// Entries in this map are generated when a PatternBindingDecl is emitted
  /// that contains a reference, and queried for each DeclRefExpr.
  llvm::DenseMap<ValueDecl*, Value*> VarLocs;
    
public:
  CFGGen(CFG &C, FuncExpr *FE);
  ~CFGGen();

  void emitProlog(FuncExpr *FE);

  /// Retun a stable reference to the current cleanup.
  CleanupsDepth getCleanupsDepth() const {
    return Cleanups.getCleanupsDepth();
  }
  
  CFGBuilder &getBuilder() { return B; }
  
  //===--------------------------------------------------------------------===//
  // Statements.
  //===--------------------------------------------------------------------===//
  
  /// emitCondition - Emit a boolean expression as a control-flow condition.
  ///
  /// \param TheStmt - The statement being lowered, for source information on
  ///        the branch.
  /// \param E - The expression to be evaluated as a condition.
  /// \param hasFalseCode - true if the false branch doesn't just lead
  ///        to the fallthrough.
  /// \param invertValue - true if this routine should invert the value before
  ///        testing true/false.
  Condition emitCondition(Stmt *TheStmt, Expr *E,
                          bool hasFalseCode = true, bool invertValue = false);
  
  
  /// emitBranch - Emit a branch to the given jump destination, threading out
  /// through any cleanups we might need to run.
  void emitBranch(JumpDest D);
  
  //===--------------------------------------------------------------------===//
  // Statements
  //===--------------------------------------------------------------------===//
  
  /// Construct the CFG components for the given BraceStmt.
  void visitBraceStmt(BraceStmt *S);
  
  /// SemiStmts are ignored for CFG construction.
  void visitSemiStmt(SemiStmt *S) {}
  
  void visitAssignStmt(AssignStmt *S);

  void visitReturnStmt(ReturnStmt *S) {
    // FIXME: Should use empty tuple for "void" return.
    Value *ArgV = S->hasResult() ? visit(S->getResult()) : (Instruction*) 0;
    B.createReturn(S, ArgV);
  }
  
  void visitIfStmt(IfStmt *S);
  
  void visitWhileStmt(WhileStmt *S);
  
  void visitDoWhileStmt(DoWhileStmt *S);
  
  void visitForStmt(ForStmt *S);
  
  void visitForEachStmt(ForEachStmt *S);
  
  void visitBreakStmt(BreakStmt *S);
  
  void visitContinueStmt(ContinueStmt *S);
  
  //===--------------------------------------------------------------------===//
  // Expressions
  //===--------------------------------------------------------------------===//
  
  Value *visitExpr(Expr *E) {
    E->dump();
    llvm_unreachable("Not yet implemented");
  }
  
  Value *visitApplyExpr(ApplyExpr *E);
  Value *visitDeclRefExpr(DeclRefExpr *E);
  Value *visitIntegerLiteralExpr(IntegerLiteralExpr *E);
  Value *visitFloatLiteralExpr(FloatLiteralExpr *E);
  Value *visitCharacterLiteralExpr(CharacterLiteralExpr *E);
  Value *visitStringLiteralExpr(StringLiteralExpr *E);
  Value *visitLoadExpr(LoadExpr *E);
  Value *visitMaterializeExpr(MaterializeExpr *E);
  Value *visitRequalifyExpr(RequalifyExpr *E);
  Value *visitFunctionConversionExpr(FunctionConversionExpr *E);
  Value *visitParenExpr(ParenExpr *E);
  Value *visitTupleExpr(TupleExpr *E);
  Value *visitScalarToTupleExpr(ScalarToTupleExpr *E);
  Value *visitSpecializeExpr(SpecializeExpr *E);
  Value *visitAddressOfExpr(AddressOfExpr *E);
  Value *visitTupleElementExpr(TupleElementExpr *E);
  Value *visitTupleShuffleExpr(TupleShuffleExpr *E);
  Value *visitNewArrayExpr(NewArrayExpr *E);
  Value *visitTypeOfExpr(TypeOfExpr *E);

  Value *emitArrayInjectionCall(Value *ObjectPtr, Value *BasePtr,
                                Value *Length, Expr *ArrayInjectionFunction);
  Value *emitTupleShuffle(Expr *E, ArrayRef<Value *> InOps,
                          ArrayRef<int> ElementMapping,
                          Expr *VarargsInjectionFunction);

  //===--------------------------------------------------------------------===//
  // Declarations
  //===--------------------------------------------------------------------===//
  
  void visitDecl(Decl *D) {
    D->dump();
    llvm_unreachable("Not yet implemented");
  }

  // ClassDecl - emitClassDecl
  // FuncDecl - emitLocalFunction
  // OneOfDecl - emitOneOfDecl
  void visitPatternBindingDecl(PatternBindingDecl *D);
  // StructDecl - emitStructDecl
    
  void visitTypeAliasDecl(TypeAliasDecl *D) {
    // No lowering support needed.
  }
    
  void visitVarDecl(VarDecl *D) {
    // We handle these in pattern binding.
  }
};
  
} // end namespace Lowering
} // end namespace swift

#endif
