# Unreal MCP Node Tools

This document provides detailed information about the Blueprint node tools available in the Unreal MCP integration.

## Overview

Node tools allow you to manipulate Blueprint graph nodes and connections programmatically, including adding event nodes, function nodes, variables, and creating connections between nodes.

## Node Tools

### add_blueprint_event_node

Add an event node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `event_name` (string) - Name of the event. Use 'Receive' prefix for standard events (e.g., 'ReceiveBeginPlay', 'ReceiveTick')
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path where the blueprint is located (default: "/Game/Blueprints")
- `rationale` (string, optional) - Design rationale for knowledge base

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
add_blueprint_event_node(
    blueprint_name="BP_MyActor",
    event_name="ReceiveBeginPlay",
    node_position=[100, 100],
    path="/Game/Blueprints"
)
```

### add_blueprint_input_action_node

Add an input action event node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `action_name` (string) - Name of the input action to respond to
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
add_blueprint_input_action_node(
    blueprint_name="BP_MyActor",
    action_name="IA_Jump",
    node_position=[200, 200]
)
```

### add_blueprint_function_node

Add a function call node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `target` (string) - Target object for the function (component name or self)
- `function_name` (string) - Name of the function to call
- `params` (object, optional) - Parameters to set on the function node
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")
- `rationale` (string, optional) - Design rationale for knowledge base

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
add_blueprint_function_node(
    blueprint_name="BP_MyActor",
    target="StaticMeshComponent",
    function_name="SetRelativeLocation",
    params={"NewLocation": [0, 0, 100]},
    node_position=[300, 300]
)
```

### connect_blueprint_nodes

Connect two nodes in a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `source_node_id` (string) - ID of the source node
- `source_pin` (string) - Name of the output pin on the source node
- `target_node_id` (string) - ID of the target node
- `target_pin` (string) - Name of the input pin on the target node
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response indicating success or failure

**Example:**
```python
connect_blueprint_nodes(
    blueprint_name="BP_MyActor",
    source_node_id="ABC123...",
    source_pin="then",
    target_node_id="DEF456...",
    target_pin="execute"
)
```

### add_blueprint_variable

Add a variable to a Blueprint.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `variable_name` (string) - Name of the variable
- `variable_type` (string) - Type of the variable (Boolean, Integer, Float, Vector, String, etc.)
- `is_exposed` (boolean, optional) - Whether to expose the variable to the editor (default: false)
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")
- `rationale` (string, optional) - Design rationale for knowledge base

**Returns:**
- Response indicating success or failure

**Example:**
```python
add_blueprint_variable(
    blueprint_name="BP_MyActor",
    variable_name="Health",
    variable_type="Float",
    is_exposed=True
)
```

### add_blueprint_get_self_component_reference

Add a node that gets a reference to a component owned by the current Blueprint.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `component_name` (string) - Name of the component to get a reference to
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
add_blueprint_get_self_component_reference(
    blueprint_name="BP_MyActor",
    component_name="StaticMeshComponent",
    node_position=[400, 400]
)
```

### add_blueprint_self_reference

Add a 'Get Self' node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
add_blueprint_self_reference(
    blueprint_name="BP_MyActor",
    node_position=[500, 500]
)
```

### find_blueprint_nodes

Find nodes in a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_type` (string, optional) - Type of node to find (Event, Function, Variable, etc.)
- `event_type` (string, optional) - Specific event type to find (BeginPlay, Tick, etc.)
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing array of found node IDs and success status

**Example:**
```python
find_blueprint_nodes(
    blueprint_name="BP_MyActor",
    node_type="Event",
    event_type="ReceiveBeginPlay"
)
```

---

## Node Manipulation Tools (New)

These tools provide advanced node manipulation capabilities for Blueprint graphs.

### set_node_pin_value

Set a default value on a node's pin.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_id` (string) - GUID of the node (from find_blueprint_nodes or node creation)
- `pin_name` (string) - Name of the pin to set (e.g., "InString" for PrintString)
- `pin_value` (string) - Value to set on the pin
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response indicating success or failure

**Supported Pin Types:**
- String / Text
- Integer
- Float / Real
- Boolean
- Name

**Example:**
```python
# Set the message for a PrintString node
set_node_pin_value(
    blueprint_name="BP_Test",
    node_id="ABC123...",
    pin_name="InString",
    pin_value="Hello World!"
)

# Set an integer value
set_node_pin_value(
    blueprint_name="BP_Test",
    node_id="DEF456...",
    pin_name="Count",
    pin_value="42"
)
```

### add_variable_get_node

Add a Variable Get node to retrieve a variable's value.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `variable_name` (string) - Name of the variable to get (must exist in blueprint)
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
# First create a variable
add_blueprint_variable(
    blueprint_name="BP_Test",
    variable_name="Health",
    variable_type="Float"
)

# Then add a get node
add_variable_get_node(
    blueprint_name="BP_Test",
    variable_name="Health",
    node_position=[200, 100]
)
```

### add_variable_set_node

Add a Variable Set node to assign a value to a variable.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `variable_name` (string) - Name of the variable to set (must exist in blueprint)
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Example:**
```python
# First create a variable
add_blueprint_variable(
    blueprint_name="BP_Test",
    variable_name="Health",
    variable_type="Float"
)

# Then add a set node
add_variable_set_node(
    blueprint_name="BP_Test",
    variable_name="Health",
    node_position=[400, 100]
)
```

### add_branch_node

Add a Branch (if/else) node for conditional logic.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the node ID and success status

**Pins:**
- **Input:**
  - `execute` - Execution input
  - `Condition` - Boolean condition to evaluate
- **Output:**
  - `True` - Execution continues here if condition is true
  - `False` - Execution continues here if condition is false

**Example:**
```python
add_branch_node(
    blueprint_name="BP_Test",
    node_position=[600, 100]
)
```

### delete_blueprint_node

Delete a node from a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_id` (string) - GUID of the node to delete
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response indicating success or failure

**Note:** All connections to/from the node will be broken before deletion.

**Example:**
```python
delete_blueprint_node(
    blueprint_name="BP_Test",
    node_id="ABC123...",
    path="/Game/Blueprints"
)
```

### move_blueprint_node

Move a node to a new position in the Blueprint graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_id` (string) - GUID of the node to move
- `position` (array) - [X, Y] new position for the node
- `path` (string, optional) - Content browser path (default: "/Game/Blueprints")

**Returns:**
- Response containing the new position and success status

**Example:**
```python
move_blueprint_node(
    blueprint_name="BP_Test",
    node_id="ABC123...",
    position=[1000, 300]
)
```

---

## Workflow Example

Here's a complete workflow example that demonstrates creating a Blueprint with variables and logic:

```python
# 1. Create a Blueprint
create_blueprint(
    name="BP_HealthSystem",
    parent_class="Actor",
    path="/Game/Blueprints"
)

# 2. Add variables
add_blueprint_variable(
    blueprint_name="BP_HealthSystem",
    variable_name="CurrentHealth",
    variable_type="Float",
    path="/Game/Blueprints"
)

add_blueprint_variable(
    blueprint_name="BP_HealthSystem",
    variable_name="MaxHealth",
    variable_type="Float",
    path="/Game/Blueprints"
)

add_blueprint_variable(
    blueprint_name="BP_HealthSystem",
    variable_name="IsDead",
    variable_type="Boolean",
    path="/Game/Blueprints"
)

# 3. Add BeginPlay event
event_result = add_blueprint_event_node(
    blueprint_name="BP_HealthSystem",
    event_name="ReceiveBeginPlay",
    node_position=[0, 0],
    path="/Game/Blueprints"
)

# 4. Add variable set node to initialize health
set_result = add_variable_set_node(
    blueprint_name="BP_HealthSystem",
    variable_name="CurrentHealth",
    node_position=[300, 0],
    path="/Game/Blueprints"
)

# 5. Set initial value on the set node
set_node_pin_value(
    blueprint_name="BP_HealthSystem",
    node_id=set_result["node_id"],
    pin_name="CurrentHealth",
    pin_value="100.0",
    path="/Game/Blueprints"
)

# 6. Connect BeginPlay to Set node
connect_blueprint_nodes(
    blueprint_name="BP_HealthSystem",
    source_node_id=event_result["node_id"],
    source_pin="then",
    target_node_id=set_result["node_id"],
    target_pin="execute",
    path="/Game/Blueprints"
)

# 7. Add branch node for death check
branch_result = add_branch_node(
    blueprint_name="BP_HealthSystem",
    node_position=[600, 0],
    path="/Game/Blueprints"
)

# 8. Add variable get node for health check
get_result = add_variable_get_node(
    blueprint_name="BP_HealthSystem",
    variable_name="CurrentHealth",
    node_position=[400, 100],
    path="/Game/Blueprints"
)

# 9. Compile the blueprint
compile_blueprint(
    blueprint_name="BP_HealthSystem",
    path="/Game/Blueprints"
)
```

---

## Error Handling

All command responses include a status field indicating whether the operation succeeded:

**Success Response:**
```json
{
  "status": "success",
  "result": {
    "node_id": "ABC123...",
    "variable_name": "Health"
  }
}
```

**Error Response:**
```json
{
  "status": "error",
  "error": "Variable not found in blueprint: Health"
}
```

---

## Type Reference

### Node Types

Common node types for the `find_blueprint_nodes` command:

- `Event` - Event nodes (BeginPlay, Tick, etc.)
- `Function` - Function call nodes
- `Variable` - Variable nodes
- `Component` - Component reference nodes
- `Self` - Self reference nodes

### Variable Types

Common variable types for the `add_blueprint_variable` command:

- `Boolean` - True/false values
- `Integer` / `Int` - Whole numbers
- `Float` - Decimal numbers
- `Vector` - 3D vector values (X, Y, Z)
- `String` - Text values

### Pin Names

Common pin names used in Blueprint nodes:

| Node Type | Input Pins | Output Pins |
|-----------|------------|-------------|
| Event | - | `then`, return values |
| Branch | `execute`, `Condition` | `True`, `False` |
| Set Variable | `execute`, variable value | `then`, variable value |
| Get Variable | - | variable value |
| Function Call | `execute`, parameters | `then`, return values |
