#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <cassert>
#include <algorithm>

using namespace std;


const int INF = 10000000;
const int MAX_VARS = 100;

class Shortest {
public:
  bool empty;
  int width;
  vector<string> vars;
  vector<int> diffs;

  vector<pair<int, int>> undo_history;
  vector<int> milestones;

  Shortest() : empty(false), width(MAX_VARS), diffs(MAX_VARS * MAX_VARS, INF) {
    for (int i = 0; i < MAX_VARS; i++)
      diffs[i * (width + 1)] = 0;
  }

  int add_var(string x) {
    assert(find(vars.begin(), vars.end(), x) == vars.end());
    assert(vars.size() < MAX_VARS);
    vars.push_back(x);
    return vars.size() - 1;
  }

  int var_index(string x) const {
    auto p = find(vars.begin(), vars.end(), x);
    assert(p != vars.end());
    return p - vars.begin();
  }

  int get_diff(int x1, int x2) const {
    if (empty)
      return -INF;
    return diffs[x1 * width + x2];
  }

  // Add inequality "x1 - x2 <= d"
  void add_constraint(string x1, string x2, int d) {
    add_constraint(var_index(x1), var_index(x2), d);
  }

  void add_constraint(int x1, int x2, int d) {
    assert(d < INF);
    assert(d > -INF);
    if (empty)
      return;
    if (d + diffs[x2 * width + x1] < 0) {
      empty = true;
      return;
    }
    if (d >= diffs[x1 * width + x2])
      return;
    assert(x1 != x2);

    change_diffs(x1 * width + x2, d);

    for (int x0 = 0; x0 < vars.size(); x0++) {
      int d0 = diffs[x0 * width + x1];
      if (d0 == INF)
        continue;
      if (d0 + d < diffs[x0 * width + x2]) {
        change_diffs(x0 * width + x2, d0 + d);
        for (int x3 = 0; x3 < vars.size(); x3++) {
          int d2 = diffs[x2 * width + x3];
          if (d2 == INF)
            continue;
          if (d0 + d + d2 < diffs[x0 * width + x3])
            change_diffs(x0 * width + x3, d0 + d + d2);
        }
      }
    }
    for (int x3 = 0; x3 < vars.size(); x3++) {
      int d2 = diffs[x2 * width + x3];
      if (d2 == INF)
        continue;
      if (d + d2 < diffs[x1 * width + x3]) {
        change_diffs(x1 * width + x3, d + d2);
        for (int x0 = 0; x0 < vars.size(); x0++) {
          int d0 = diffs[x0 * width + x1];
          if (d0 == INF)
            continue;
          if (d0 + d + d2 < diffs[x0 * width + x3])
            change_diffs(x0 * width + x3, d0 + d + d2);
        }
      }
    }
  }

  void change_diffs(int index, int d) {
    assert(diffs[index] > d);
    undo_history.push_back(make_pair(index, diffs[index]));
    diffs[index] = d;
  }

  void push() {
    milestones.push_back(undo_history.size());
  }

  void pop() {
    assert(!milestones.empty());
    int m = milestones.back();
    milestones.pop_back();
    assert(undo_history.size() >= m);
    while (undo_history.size() > m) {
      auto p = undo_history.back();
      undo_history.pop_back();
      diffs[p.first] = p.second;
    }
  }

  vector<int> any_solution() {
    push();
    int x1 = 0;
    for (int x2 = 1; x2 < vars.size(); x2++) {
      int d = diffs[x1 * width + x2];
      int rd = diffs[x2 * width + x1];
      if (d == INF || rd == INF || d + rd > 0) {
        int new_d = (d != INF) ? d : (rd != INF) ? -rd : 0;
        add_constraint(x1, x2, new_d);
        add_constraint(x2, x1, -new_d);
      }
    }
    vector<int> result;
    for (int x = 0; x < vars.size(); x++) {
      assert(get_diff(0, x) + get_diff(x, 0) == 0);
      result.push_back(get_diff(x, 0));
    }
    assert(!empty);
    pop();
    int m = *min_element(result.begin(), result.end());
    for (int x = 0; x < result.size(); x++)
      result[x] -= m;
    return result;
  }
};


ostream& operator<<(ostream& out, Shortest& s) {
  if (s.empty) {
    out << "EMPTY" << endl;
    return out;
  }
  int cnt = 0;
  for (int x1 = 0; x1 < s.vars.size(); x1++) {
    for (int x2 = x1 + 1; x2 < s.vars.size(); x2++) {
      int d = s.diffs[x1 * s.width + x2];
      int rd = s.diffs[x2 * s.width + x1];
      if (d == INF && rd == INF)
        continue;
      cnt++;
      if (d != INF) {
        if (rd != INF)
          out << -rd << " <= ";
        out << s.vars[x1] << " - " << s.vars[x2];
        out << " <= " << d;
      } else {
        out << s.vars[x2] << " - " << s.vars[x1];
        out << " <= " << rd;
      }

      out << endl;
    }
  }
  if (cnt == 0)
    out << "ANYTHING" << endl;
}