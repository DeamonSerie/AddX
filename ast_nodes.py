from dataclasses import dataclass
from typing import Optional

@dataclass
class ASTNode:
    pass

@dataclass
class ProgramNode(ASTNode):
    statements: list[ASTNode]

@dataclass
class FunctionDefNode(ASTNode):
    name: str
    params: list[tuple[str, str]]
    return_type: str
    body: list[ASTNode]

@dataclass
class ClassDefNode(ASTNode):
    name: str
    base_class: Optional[str]
    body: list[ASTNode]

@dataclass
class VarDeclNode(ASTNode):
    name: str
    var_type: Optional[str]
    value: Optional[ASTNode]

@dataclass
class AssignmentNode(ASTNode):
    target: ASTNode  # Can be IdentifierNode or AttributeAccessNode
    value: ASTNode

@dataclass
class IfNode(ASTNode):
    condition: ASTNode
    then_branch: list[ASTNode]
    else_branch: Optional[list[ASTNode]]
    elif_branches: list[tuple[ASTNode, list[ASTNode]]]

@dataclass
class WhileNode(ASTNode):
    condition: ASTNode
    body: list[ASTNode]

@dataclass
class ForNode(ASTNode):
    var_name: str
    start: Optional[ASTNode]
    end: ASTNode
    step: Optional[ASTNode]
    body: list[ASTNode]

@dataclass
class ReturnNode(ASTNode):
    value: Optional[ASTNode]

@dataclass
class PrintNode(ASTNode):
    args: list[ASTNode]

@dataclass
class BinaryOpNode(ASTNode):
    op: str
    left: ASTNode
    right: ASTNode

@dataclass
class UnaryOpNode(ASTNode):
    op: str
    operand: ASTNode

@dataclass
class NumberNode(ASTNode):
    value: float

@dataclass
class StringNode(ASTNode):
    value: str

@dataclass
class SimpleNode(ASTNode):
    value: str

@dataclass
class SimpleNumNode(ASTNode):
    value: float

@dataclass
class BoolNode(ASTNode):
    value: bool

@dataclass
class NoneNode(ASTNode):
    pass

@dataclass
class IdentifierNode(ASTNode):
    name: str

@dataclass
class ListNode(ASTNode):
    elements: list[ASTNode]

@dataclass
class IndexAccessNode(ASTNode):
    object: ASTNode
    index: ASTNode

@dataclass
class AttributeAccessNode(ASTNode):
    object: ASTNode
    attribute: str

@dataclass
class CallNode(ASTNode):
    func: ASTNode
    args: list[ASTNode]

@dataclass
class BreakNode(ASTNode):
    pass

@dataclass
class ContinueNode(ASTNode):
    pass

@dataclass
class NullptrNode(ASTNode):
    pass

@dataclass
class AddressOfNode(ASTNode):
    variable: str

@dataclass
class DereferenceNode(ASTNode):
    pointer: ASTNode

@dataclass
class SizeofNode(ASTNode):
    type_name: Optional[str]
    expr: Optional[ASTNode]

@dataclass
class StaticVarNode(ASTNode):
    name: str
    var_type: str
    value: Optional[ASTNode]

@dataclass
class ConstNode(ASTNode):
    name: str
    var_type: str
    value: ASTNode

@dataclass
class NewNode(ASTNode):
    type_name: str
    args: list[ASTNode]

@dataclass
class DeleteNode(ASTNode):
    expr: ASTNode

@dataclass
class InheritNode(ASTNode):
    class_name: str
    inherited_type: str

@dataclass
class ModanNode(ASTNode):
    name: str
    target: str
    lines: list[int]