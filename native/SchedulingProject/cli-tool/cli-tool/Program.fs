// Learn more about F# at https://fsharp.org
// See the 'F# Tutorial' project for more help.

open System.IO
open System.Diagnostics

open System.Collections.Generic

open Microsoft.FSharp.Quotations
open Microsoft.FSharp.Quotations.Patterns
open Microsoft.FSharp.Quotations.DerivedPatterns
open Microsoft.FSharp.Quotations.ExprShape
open FSharp.Quotations.Evaluator


type ValType = int
type IdleTime = ValType
type JobLength = ValType
type ServerJobs = JobLength list
type AllJobs = ServerJobs list
type SyncPoints = ValType list
type BruteForceSolution = SyncPoints * IdleTime

let valInf: ValType = Core.int32.MaxValue

type Scenario =
    val syncPoints: SyncPoints
    val jobs: AllJobs
    val t: ValType
    val useT: bool
    new(syncPoints, jobs, t, useT) = {
        syncPoints = syncPoints;
        jobs = jobs;
        t = t;
        useT = useT;
    }

let loadScenario (path: string) : Scenario =
    printfn "Loading scenario at \"%s\"" path
    use stream = System.IO.File.OpenText path
    let version = stream.ReadLine()
    printfn "version: \"%s\"" version

    let readInt () = stream.ReadLine() |> int
    let readInts n = List.ofSeq (Seq.init n (fun i -> readInt ()))
    let readServer () = 
        let jobCount = readInt ()
        readInts jobCount
    
    let serverCount = readInt ()
    printfn "serverCount: \"%i\"" serverCount
    let jobs = List.ofSeq (Seq.init serverCount (fun i -> readServer ()))
         
    let syncPointCount = readInt ()
    printfn "syncPointCount: \"%i\"" syncPointCount
    let syncPoints = List.ofSeq (readInts syncPointCount)

    let t = readInt ()
    printfn "t: \"%i\"" t
    
    let useT = readInt ()
    printfn "useT: \"%i\"" useT

    printfn "syncPointCount: \"%i\"" syncPointCount
    new Scenario(syncPoints, jobs, t, useT > 0)
    
let saveScenario (scenario: Scenario) (path: string) : unit =

    printfn "saving scenario at \"%s\"" path
    use stream = System.IO.File.CreateText path

    stream.WriteLine "v3"

    stream.WriteLine (scenario.jobs.Length.ToString())
    for serverJobs in scenario.jobs do
        stream.WriteLine (serverJobs.Length.ToString())
        for job in serverJobs do
            stream.WriteLine (job.ToString())
            
    stream.WriteLine (scenario.syncPoints.Length.ToString())
    for syncPoint in scenario.syncPoints do
        stream.WriteLine (syncPoint.ToString())

    stream.WriteLine (scenario.t.ToString())
    stream.WriteLine ((if scenario.useT then 1 else 0).ToString())

    stream.Close()

let sumIdleTime (a: IdleTime) (b: IdleTime) : IdleTime =
    if a = valInf then valInf
    elif b = valInf then valInf
    else a + b

// Simulatneously computes idle time before "time" and computes remaining work to be done
let jobSplitter (time: ValType) (t: ValType) (useT: bool) : ServerJobs -> IdleTime * ServerJobs =
    fun jobs ->
        match jobs with
        | [] -> (0, [])
        | firstJob :: remainingJobs ->
            if firstJob <= time && remainingJobs.IsEmpty then (0, remainingJobs)
            elif firstJob <= time then (time - firstJob, remainingJobs)
            else (0, firstJob - time :: remainingJobs)

// Idle time and optimal syncPoints given a time for first sync point
let rec bestByTime (availableSyncPoints: int) (serverJobs: AllJobs) (t: ValType) (useT: bool) : ValType -> BruteForceSolution =
    if availableSyncPoints = 1
    then
        fun time ->
            let jobs = serverJobs |> List.map (jobSplitter time t useT)

            let jobIdleTime (jobs: ServerJobs) =
                if jobs.IsEmpty
                then t
                else (t - jobs.Head)

            let remainingIdleTime =
                jobs
                |> List.map (snd >> jobIdleTime >> (max 0))
                |> List.sum

            ([time], sumIdleTime (jobs |> List.sumBy fst) remainingIdleTime)
    else
        fun time ->
            let jobs = serverJobs |> List.map (jobSplitter time t useT)

            let optimalSyncPoints, remainingIdleTime = minimizeBruteForce (availableSyncPoints - 1) (jobs |> List.map snd) (t - time) useT

            let syncPoints = time :: (optimalSyncPoints |> List.map (fun p -> p + time))

            (syncPoints, sumIdleTime (jobs |> List.sumBy fst) remainingIdleTime)

and minimizeBruteForce (availableSyncPoints: int) (serverJobs: AllJobs) (t: ValType) (useT: bool) : BruteForceSolution =

    let lessThanT = (>) t

    let timeValues = 
        serverJobs
        |> List.filter (not << List.isEmpty)
        |> List.map (fun jobs -> jobs.Head)
        |> if useT then List.filter lessThanT else id
        |> List.distinct

    if timeValues.IsEmpty && availableSyncPoints = 0 then ([], 0)
    elif not timeValues.IsEmpty && availableSyncPoints = 0 then ([], valInf)
    elif timeValues.IsEmpty then ([], 0)
    else
        //try puting the first sync point a each time value. Find the one with least idle time
        timeValues
            |> List.map (bestByTime availableSyncPoints serverJobs t useT)
            |> List.minBy fst

let idleTime (scenario: Scenario) : IdleTime =

    let rec idleTimeForRemaining (serverJobs: AllJobs) (syncPoints: SyncPoints) : IdleTime =
        match syncPoints with
        | [] ->
            if serverJobs |> List.forall (not << List.isEmpty)
            then valInf
            else 0
        | time :: remaining ->
            let jobs = serverJobs |> List.map (jobSplitter time scenario.t scenario.useT)
            let remainingIdleTime = idleTimeForRemaining (jobs |> List.map snd) remaining
            sumIdleTime (jobs |> List.sumBy fst) remainingIdleTime

    idleTimeForRemaining scenario.jobs scenario.syncPoints

let runIdleTimeCalculator path = 
    let scenario = loadScenario path
    printfn "idle time: %i" (idleTime scenario)

let runBruteOptimizer path =

    let scenario = loadScenario path

    printfn "original syncPoints: %A" scenario.syncPoints

    let optimalSyncPoints, minimumIdleTime = minimizeBruteForce scenario.syncPoints.Length scenario.jobs scenario.t scenario.useT
    
    let sw = new Stopwatch()
    sw.Start()
    for i = 1 to 1000 do
        let _, _ = minimizeBruteForce scenario.syncPoints.Length scenario.jobs scenario.t scenario.useT
        ()
    sw.Stop()

    printfn "Time to solve: %A" sw.ElapsedMilliseconds

    let optimalSolution = new Scenario(optimalSyncPoints, scenario.jobs, scenario.t, scenario.useT)

    saveScenario optimalSolution (String.concat "_" [path; "solution"])

    printfn "optimal syncPoints: %A" optimalSolution.syncPoints

type PlaneIndex = uint32
type CoefficientIndex = uint32
type ExpressionIndex = uint32

type UndefinedVar =
    val public columnIndex: CoefficientIndex
    new (columnIndex) = {
        columnIndex = columnIndex
    }

//type BinaryBooleanOp = 
//    | GreaterThan
//    | LessThan
//    | GreaterThanEqual
//    | LessThanEqual

//type LinearExpr =
//    | LinearComb of LinearCombination
//    | BinaryBoolean of a: LinearExpr * op: BinaryBooleanOp * b: LinearExpr
//    | GreaterThenElse of cond: LinearExpr * greater: LinearExpr * lessEqual: LinearExpr

//and LinearCombination = 
//    val coefficeints: ValType array
//    member public this.IsConstant = 
//        this.coefficeints.[1..] |> Array.forall ((=) 0)

//and PieceWiseLinearExpr =
//    val boundaries: Set<PlaneIndex * ExpressionIndex>


type Solver() =

    let addVariable(): UndefinedVar =
        UndefinedVar(0u) //TODO: implement

    member public this.minimize (objFunc: Expr) ([<System.ParamArray>] args: obj[]) : bool =

        let variableStack = new Stack<Var list>()

        //let rec evalExpr (expr: Expr) : LinearExpr =
        //    match expr with
        //    | SpecificCall <@@ (<) @@> (_, _, exprList) ->
        //        BinaryBoolean (evalExpr exprList.Head, LessThan, evalExpr exprList.Tail.Head)
        //    | IfThenElse(cond, ifTrueExpr, elseExpr) ->
        //        //return an expression that splits the domain using "cond"
        //        let condVal = evalExpr cond
        //        if condVal.IsConstant
        //        then
        //            if 
        //        else GreaterThenElse (evalExpr cond, evalExpr ifTrueExpr, evalExpr elseExpr)
        //    | _ -> printfn "Not recognized"

        let ifHandler (cond: ValType) (trueVal: Lazy<ValType>) (falseVal: Lazy<ValType>) : ValType =
            0

        let rec mapExpr (expression: Expr) : Expr =
            match expression with
            | IfThenElse(cond, trueExpr, falseExpr) ->
                // repalce if then else expressions with function call
                let nTrueExpr = mapExpr trueExpr
                let nFalseExpr = mapExpr falseExpr
                <@@ ifHandler %%cond (lazy (%%nTrueExpr)) (lazy (%%nFalseExpr)) @@>
            | ShapeVar var -> Expr.Var var
            | ShapeLambda (var, expr) -> Expr.Lambda (var, mapExpr expr)
            | ShapeCombination(shapeComboObject, exprList) ->
                RebuildShapeCombination(shapeComboObject, List.map mapExpr exprList)
        
        let processRootLambda (objFuncParams: Var list list) (body: Expr) =
            printfn "lambda provided"
            let mutable i = 0
            for subParams in objFuncParams do
                printfn "sub param %i" i
                i <- i + 1
                for p in subParams do
                    printfn "param %A" p.Name
            printfn "finished"

        // evaluate and build expression tree that references defined and undefined variables
        match objFunc with
        | Lambdas(pl, body) ->
            let mappedExpr = mapExpr objFunc
            let res = mappedExpr |> QuotationEvaluator.Evaluate
            processRootLambda pl body
            true
        | _ ->
            printfn "No lambda provided"
            false

let runBruteOptimizerLP path =
    
    let scenario = loadScenario path
    
    let objectiveFunc = <@ fun (serverJobs: AllJobs) (syncPoints: SyncPoints) (t: ValType) ->
    
        let isIncreasing (arr: ValType list) =
            arr.Length < 2 || (List.forall2 (fun a b -> a < b) arr.[..(arr.Length-2)] arr.[1..])
        
        let rec computeSingleServer (syncPoints: SyncPoints) (t: ValType) (jobs: ServerJobs) : ValType =
            match (syncPoints, jobs) with
            | (_, []) -> t
            | (_, jobs) when jobs.Head >= t -> 0
            | (_, [job]) -> t - job
            | ([], jobs) -> t - jobs.Head
            | (syncPoint :: rSyncPoints, jobs) ->
                let tSyncPoints = lazy (rSyncPoints |> List.map (fun sp -> sp - syncPoint))
                let tT = lazy (t - syncPoint)
                if syncPoint < jobs.Head
                then
                    let tJobs = (jobs.Head - syncPoint) :: jobs.Tail
                    computeSingleServer (tSyncPoints.Force()) (tT.Force()) tJobs
                else
                    if syncPoint < (t - jobs.[1])
                    then
                        let tJobs = jobs.Tail |> List.map (fun j -> j - syncPoint)
                        t - jobs.Head - jobs.[1] + (computeSingleServer (tSyncPoints.Force()) (tT.Force()) tJobs)
                    else
                        t - jobs.Head - tT.Force()
    
        if (isIncreasing syncPoints)
        then
            let idleTime =
                serverJobs
                |> List.map (computeSingleServer syncPoints t)
                |> List.sum
            Some(idleTime)
        else None @>
    
    let solver = new Solver()

    let optimal = solver.minimize objectiveFunc [scenario.jobs, scenario.syncPoints, scenario.t]

    None

[<EntryPoint>]
let main argv =

    runBruteOptimizerLP "pladd_instance_07"

    let x = stdin.Read()
    
    0 // return an integer exit code
