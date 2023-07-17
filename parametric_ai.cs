using System.Collections.Generic;
using System.Linq;

public interface ParametricAPI<V, R> {
  R Distance(R ratio, (V, V)e);
  R ZeroCancel(List<(V, V)> cycle);
}

public class MaxParametricSolver<V, R> {
  private readonly Dictionary<V, Dictionary<V, dynamic>> gra;
  private readonly ParametricAPI<V, R> omega;
  private readonly NegCycleFinder<V, R> ncf;

  public MaxParametricSolver(Dictionary<V, Dictionary<V, dynamic>> gra,
                             ParametricAPI<V, R> omega) {
    this.gra = gra;
    this.omega = omega;
    this.ncf = new NegCycleFinder<V, R>(gra);
  }

  public (R, List<(V, V)>) Run(Dictionary<V, R> dist, R ratio) {
    R r_min = ratio;
    List<(V, V)> c_min = new List<(V, V)>();
    List<(V, V)> cycle = new List<(V, V)>();
    while (true) {
      foreach (var ci in ncf.FindNegCycle(dist,
                                          e => omega.Distance(ratio, e))) {
        R ri = omega.ZeroCancel(ci);
        if (r_min.CompareTo(ri) > 0) {
          r_min = ri;
          c_min = ci;
        }
      }
      if (r_min.CompareTo(ratio) >= 0) {
        break;
      }
      cycle = c_min;
      ratio = r_min;
    }
    return (ratio, cycle);
  }
}

public class NegCycleFinder<V, R> {
  private readonly Dictionary<V, Dictionary<V, dynamic>> gra;

  public NegCycleFinder(Dictionary<V, Dictionary<V, dynamic>> gra) {
    this.gra = gra;
  }

  public List<List<(V, V)>> FindAllNegCycles() {
    var dist = new Dictionary<V, R>();
    var prev = new Dictionary<V, (V, V)>();
    foreach (var u in gra.Keys) {
      dist[u] = default(R);
      prev[u] = (default(V), default(V));
    }
    var cycle = new List<(V, V)>();
    foreach (var u in gra.Keys) {
      if (dist[u].CompareTo(default(R)) != 0) {
        continue;
      }
      var v = u;
      while (true) {
        var e = (default(V), default(V));
        foreach (var (to, w) in gra[v]) {
          if (dist[to].CompareTo(dist[v].Add(w)) > 0) {
            dist[to] = dist[v].Add(w);
            prev[to] = (v, to);
            e = (v, to);
          }
        }
        if (e.Item1 == null) {
          break;
        }
        v = e.Item2;
        if (cycle.Contains(e)) {
          var idx = cycle.IndexOf(e);
          return new List<List<(V, V)>> { cycle.Skip(idx).ToList() };
        }
        cycle.Add(e);
      }
      foreach (var e in cycle) {
        if (dist[e.Item2].CompareTo(default(R)) != 0) {
          continue;
        }
        var to = e.Item2;
        while (true) {
          to = prev[to].Item1;
          if (dist[to].CompareTo(default(R)) != 0) {
            break;
          }
          dist[to] = default(R);
        }
      }
      cycle.Clear();
    }
    return new List<List<(V, V)>>();
  }

  public List<List<(V, V)>> FindNegCycle(Dictionary<V, R> dist,
                                         System.Func<(V, V), R> weight) {
    var gra = new Dictionary<V, Dictionary<V, dynamic>>();
    foreach (var (u, v) in dist.Keys) {
      if (!gra.ContainsKey(u)) {
        gra[u] = new Dictionary<V, dynamic>();
      }
      gra[u][v] = weight((u, v));
    }
    foreach (var (u, to) in gra.Keys.SelectMany(
                 u => gra[u].Keys.Select(to => (u, to)))) {
      gra[u][to] = gra[u][to].Subtract(dist[u]).Add(dist[to]);
    }
    return new NegCycleFinder<V, R>(gra).FindAllNegCycles();
  }
}
