#!C:/Python27/

# Copyright 2016, Gurobi Optimization, Inc.

from gurobipy import *

def printvar(name, value):
    print('{0} = {1}'.format(name, value))
    return

def preprocessing(T,K,serverjobs):
    T = T + 1
    if len(serverjobs) > (K+1):
        serverjobs = serverjobs[:K+1]
    if len(serverjobs) <= K and sum(serverjobs) < T:
        return 'Not enought jobs on this server'
    if serverjobs[0] > T:
        serverjobs[0] = T
        serverjobs = serverjobs[:1]
        K = 0
    totallength = serverjobs[0]
    K = len(serverjobs) - 1
    for i in range (1, K+1):
        if totallength + serverjobs[i] > T:
            serverjobs[i] = T - totallength
            serverjobs = serverjobs[:i+1]
            if serverjobs[i] == 0:
                serverjobs = serverjobs[:i]
            break
        totallength = totallength + serverjobs[i]
    return serverjobs

try:


    length =  [[2,8,2,2,8,2],[4,6,4,4,6,4],[6,6,6,4]]
    T = 20
    K = 5

    M = len(length)
    for i in range (M):
        print('{3}{0}{1} = {2}'.format('server ', i, length[i],"Before preprocessing  "))
    for i in range (M):
        length[i] = preprocessing(T,K,length[i])
    for i in range (M):
        print('{3}{0}{1} = {2}'.format('server ', i, length[i],"After preprocessing  "))
    printvar('T',T)
    printvar('K',K)
    sumOfLengthServerJobs = []
    for i in range (M):
        sumOfLengths = sum(length[i])
        sumOfLengthServerJobs.append(sumOfLengths)
        print('{0}{1} = {2}'.format('length of jobs on server', i, sumOfLengths))

    # Create a new model
    pladdLPM = Model("model0")

    z = []
    zSum = LinExpr(0.0)
    for m in range(M):
        zRow = []
        for t in range(T+1):
            varName = 'Z_m{0}_t{1}'.format(m, t)
            idle = pladdLPM.addVar(lb=0.0, ub=1.0, vtype=GRB.BINARY, name=varName)
            zSum.addTerms(1.0, idle)
            zRow.append(idle)
        z.append(zRow)

    x = []
    for m in range(M):
        n = len(length[m])
        jobStartTimes = []
        for j in range(n):
            thisJobStarts = []
            for t in range(T+sumOfLengthServerJobs[m]+1):
                thisJobStarts.append(pladdLPM.addVar(lb=0.0, ub=1.0, vtype=GRB.BINARY, name='X_m{0}_j{1}_t{2}'.format(m, j, t)))
            jobStartTimes.append(thisJobStarts)
        x.append(jobStartTimes)

    y = []
    for t in range(T+1):
        y.append(pladdLPM.addVar(lb=0.0, ub=1.0, vtype=GRB.BINARY, name='Y_t{0}'.format(t)))

    pladdLPM.update()
    pladdLPM.setObjective(zSum, GRB.MINIMIZE)

    for m in range(M):
        n = len(length[m])
        for j in range(n):
            pladdLPM.addConstr(quicksum(x[m][j]) == 1.0, 'C: every job scheduled for server {0}'.format(m))

    # For a job j, jobs with a greater index much start after j ends
    for m in range(M):
        n = len(length[m])
        for i in range(n):
            for j in range(n):
                if (i > j):
                    for t in range(T+sumOfLengthServerJobs[m]+1):
                        ub = t+length[m][j] - 1 + 1
                        ub = min(ub, T+sumOfLengthServerJobs[m]+1)
                        ub1 = max(0,t-length[m][j]+1)
                        pladdLPM.addConstr(quicksum(x[m][j][t:T+1]) + quicksum(x[m][i][:ub]) <= 1.0, 'C: jobs after job{0} must start after job{1} ends for server{2}'.format(j, j, m))

    # Jobs with an index less than j must end before j starts
    for m in range(M):
        n = len(length[m])
        for i in range(n):
            for j in range(n):
                if (i < j):
                    for t in range(T+sumOfLengthServerJobs[m]+1):
                        lb = t-length[m][i]+1
                        lb = max(lb, 0)
                        ub2 = max(0,t-length[m][j]+1)
                        pladdLPM.addConstr(quicksum(x[m][j][0:t+1]) + quicksum(x[m][i][lb:]) <= 1.0, 'C: jobs before job{0} must end before job{1} starts for server{2}'.format(j, j, m))


    # Calculate idle time
    for m in range(M):
        n = len(length[m])
        for t in range(T+1):
            expr = LinExpr(0.0)
            expr.addTerms(1.0, z[m][t])
            for j in range(n):
                lb = t - length[m][j] + 1
                ub = t+1
                lb = max(lb, 0)
                expr.add(quicksum(x[m][j][lb:ub]))
            pladdLPM.addConstr(expr >= 1.0, 'C: idle time constaint for m{0}, t{1}'.format(m, t))


    # Jobs may only start at the takes, which are budgeted
    for m in range(M):
        n = len(length[m])
        for t in range(T+1):
            expr = LinExpr(0.0)
            for j in range(n):
                expr.addTerms(1.0, x[m][j][t])
            pladdLPM.addConstr(expr <= y[t], 'C: jobs can only start at take{0} for server {1}'.format(t, m))

    pladdLPM.addConstr(quicksum(y) <= (K+1), 'C: sum(y) <= k')

    pladdLPM.addConstr(y[0] == 1, 'First jobs on all servers start at time 0')

    for m in range(M):
        pladdLPM.addConstr(x[m][0][0] == 1, 'First job starts at time 0 on server {0}'.format(m))


    pladdLPM.optimize()



    print('z')
    for m in range(M):
        print(''.join(['['+str(z[m][i].x)+']' for i in range(T+1)]))


    for m in range(M):
        print('x_m{0}'.format(m))
        for j in range(len(length[m])):
            print(''.join(['[{:.3f}]'.format(x[m][j][i].x) for i in range(T+sumOfLengthServerJobs[m]+1)]))

    print('y')
    for t in range(T+1):
        print('[' + str(y[t].x) +']')

except GurobiError as err:
    print('Gurobi ERROR({0}): {1}'.format(err.errno, err.message))

except AttributeError as err:
    print('Encountered an attribute error: {0}'.format(err))
