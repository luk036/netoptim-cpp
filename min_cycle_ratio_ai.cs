using System.Collections.Generic;
using System.Linq;
using System;

public class MinCycleRatioSolver<V, R>
    where R : struct, IComparable<R>, IEquatable<R>, IConvertible {
    private readonly Dictionary<V, Dictionary<V, dynamic>> gra;
    public MinCycleRatioSolver(Dictionary<V, Dictionary<V, dynamic>> gra) {
        this.gra = gra;
    }
    public (R, List<(V, V)>) Run(Dictionary<V, R> dist, R r0) {
        var omega = new CycleRatioAPI<V, R>(gra, r0.GetType());
        var solver = new MaxParametricSolver<V, R>(gra, omega);
        var (ratio, cycle) = solver.Run(dist, r0);
        return (ratio, cycle);
    }
}

public class CycleRatioAPI<V, R>
    where R : struct, IComparable<R>, IEquatable<R>, IConvertible {
    private readonly Dictionary<V, Dictionary<V, dynamic>> gra;
    private readonly Type T;
    public CycleRatioAPI(Dictionary<V, Dictionary<V, dynamic>> gra, Type T) {
        this.gra = gra;
        this.T = T;
    }
    public R Distance(R ratio, (V, V)e) {
        var (u, v) = e;
        return (R)(dynamic)(gra[u][v]["cost"] - ratio * gra[u][v]["time"]);
    }
    public R ZeroCancel(List<(V, V)> cycle) {
        var totalCost = cycle.Sum(e => gra[e.Item1][e.Item2]["cost"]);
        var totalTime = cycle.Sum(e => gra[e.Item1][e.Item2]["time"]);
        return (R)(dynamic)(Convert.ChangeType(totalCost, T) / totalTime);
    }
}

public static class Extensions {
    public static void
    SetDefault<V, D>(this Dictionary<V, Dictionary<V, dynamic>> gra,
                     string weight, D value)
        where D : struct, IComparable<D>, IEquatable<D>, IConvertible {
        foreach (var u in gra.Keys.ToList()) {
            foreach (var v in gra[u].Keys.ToList()) {
                if (!gra[u][v].ContainsKey(weight)) {
                    gra[u][v][weight] = value;
                }
            }
        }
    }
}
