from gurobipy import *
m = Model("test")
x = m.addVar(obj=3000, vtype = , name="x")
x = m.addVar(obj=3000, vtype = BINARY, name="x")
m.modelSense = GRB.MINIMIZE
m.update()
pladdLPM.setObjective(11005.34-120.24*y-141053*x, GRB.MINIMIZE)
m.addConstr() <=)
result = m.optimize()
print(result)
