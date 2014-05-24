
struct SearchState {
  vector<bool> concrete;
  vector<vector<Box>> candidates;
  vector<bool> taken_boxes;
};


class Packer {
public:
  Coord size;
  vector<Box> *boxes;
  vector<BoxPlacement> symbolic_bps;

  Shortest xs, ys;
  int x0, y0;

  BoxPlacement make_symbolic_placement(string name) {
    BoxPlacement b;
    b.bottom_left.X = xs.add_var(name + "x1");
    b.bottom_left.Y = ys.add_var(name + "y1");
    b.top_right.X = xs.add_var(name + "x2");
    b.top_right.Y = ys.add_var(name + "y2");

    // TODO use size upper and lower bounds
    xs.add_constraint(b.bottom_left.X, b.top_right.X, -1);
    ys.add_constraint(b.bottom_left.Y, b.top_right.Y, -1);

    xs.add_constraint(b.top_right.X, b.bottom_left.X, 1000);
    ys.add_constraint(b.top_right.Y, b.bottom_left.Y, 1000);

    xs.add_constraint(x0, b.bottom_left.X, 0);
    ys.add_constraint(y0, b.bottom_left.Y, 0);

    return b;
  }

  void touches_bottom(BoxPlacement sbp) {
    ys.add_constraint(sbp.bottom_left.Y, y0, 0);
    ys.add_constraint(y0, sbp.bottom_left.Y, 0);
    xs.add_constraint(sbp.bottom_left.X, x0, size.X);
  }

  void doesnt_touch_left(BoxPlacement sbp) {
    xs.add_constraint(x0, sbp.bottom_left.X, -1);
  }

  void touches_left(BoxPlacement sbp) {
    BoxPlacement left;
    xs.add_constraint(sbp.bottom_left.X, x0, 0);
    xs.add_constraint(y0, sbp.bottom_left.X, 0);
    ys.add_constraint(sbp.bottom_left.Y, y0, size.Y);
  }

  void doesnt_touch_bottom(BoxPlacement sbp) {
    ys.add_constraint(y0, sbp.bottom_left.Y, -1);
  }

  void h_touch(BoxPlacement b1, BoxPlacement b2) {
    xs.add_constraint(b1.top_right.X, b2.bottom_left.X, 0);
    xs.add_constraint(b2.bottom_left.X, b1.top_right.X, 0);
    ys.add_constraint(b1.bottom_left.Y, b2.top_right.Y, 0);
    ys.add_constraint(b2.bottom_left.Y, b1.top_right.Y, 0);
  }

  void v_touch(BoxPlacement b1, BoxPlacement b2) {
    xs.add_constraint(b1.bottom_left.X, b2.top_right.X, 0);
    xs.add_constraint(b2.bottom_left.X, b1.top_right.X, 0);
    ys.add_constraint(b1.top_right.Y, b2.bottom_left.Y, 0);
    ys.add_constraint(b2.bottom_left.Y, b1.top_right.Y, 0);
  }

  void h_space(BoxPlacement b1, BoxPlacement b2) {
    xs.add_constraint(b1.top_right.X, b2.bottom_left.X, -1);
  }

  void v_space(BoxPlacement b1, BoxPlacement b2) {
    ys.add_constraint(b1.top_right.Y, b2.bottom_left.Y, -1);
  }

  /*vector<Box> matching_candidates() {

  }*/

  bool strengthen_constraints(SearchState &ss) {
    assert(!xs.empty && !ys.empty);
    assert(symbolic_bps.size() == ss.candidates.size());
    for (int i = 0; i < symbolic_bps.size(); i++) {
      if (ss.concrete[i]) {
        assert(ss.candidates[i].size() == 1);
        assert(ss.taken_boxes[ss.candidates[i].front().index]);
        continue;
      }
      auto sbp = symbolic_bps[i];

      int max_w = xs.get_diff(sbp.top_right.X, sbp.bottom_left.X);
      int min_w = -xs.get_diff(sbp.bottom_left.X, sbp.top_right.X);
      int max_h = ys.get_diff(sbp.top_right.Y, sbp.bottom_left.Y);
      int min_h = -ys.get_diff(sbp.bottom_left.Y, sbp.top_right.Y);

      vector<Box> new_candidates;
      for (const auto &cand : ss.candidates[i]) {
        if (cand.a >= min_w && cand.a <= max_w &&
            cand.b >= min_h && cand.b <= max_h &&
            !ss.taken_boxes[cand.index]) {
          new_candidates.push_back(cand);
        }
      }

      min_w = INF;
      min_h = INF;
      max_w = -INF;
      max_h = -INF;

      //assert(!new_candidates.empty());
      if (new_candidates.empty())
        return false;

      for (auto cand : new_candidates) {
        min_w = min(min_w, cand.a);
        min_h = min(min_h, cand.b);
        max_w = max(max_w, cand.a);
        max_h = max(max_h, cand.b);
      }

      ss.candidates[i] = new_candidates;
      xs.add_constraint(sbp.top_right.X, sbp.bottom_left.X, max_w);
      xs.add_constraint(sbp.bottom_left.X, sbp.top_right.X, -min_w);
      if (xs.empty)
        return false;
      ys.add_constraint(sbp.top_right.Y, sbp.bottom_left.Y, max_h);
      ys.add_constraint(sbp.bottom_left.Y, sbp.top_right.Y, -min_h);
      if (ys.empty)
        return false;
    }
    return true;
  }

  bool solve(vector<Box> *boxes) {
    this->boxes = boxes;

    SearchState ss;
    ss.concrete = vector<bool>(symbolic_bps.size(), false);
    vector<Box> all_candidates;
    for (auto box : *boxes) {
      all_candidates.push_back(box);
      if (box.a != box.b) {
        swap(box.a, box.b);
        all_candidates.push_back(box);
      }
    }
    ss.candidates = vector<vector<Box>>(symbolic_bps.size(), all_candidates);
    ss.taken_boxes = vector<bool>(1000, false);

    solution_found = false;

    if (xs.empty || ys.empty)
      return false;
    if (!strengthen_constraints(ss))
      return false;

    cnt = 0;
    rec(ss);
    if (cnt > 100)
      cerr << cnt << " rec calls" << endl;

    return solution_found;
  }

  vector<BoxPlacement> place() {
    assert(solution_found);
    assert(concrete_xs[x0] == 0);
    assert(concrete_ys[y0] == 0);

    vector<BoxPlacement> result;
    for (int i = 0; i < symbolic_bps.size(); i++) {
      auto sbp = symbolic_bps[i];
      BoxPlacement bp;
      bp.index = box_indices[i];
      bp.bottom_left = {
          concrete_xs[sbp.bottom_left.X],
          concrete_ys[sbp.bottom_left.Y]};
      bp.top_right = {
          concrete_xs[sbp.top_right.X],
          concrete_ys[sbp.top_right.Y]};
      result.push_back(bp);
    }

    // remove used boxes from the list
    auto p = remove_if(boxes->begin(), boxes->end(), [this](Box b) {
      return find(box_indices.begin(), box_indices.end(), b.index) != box_indices.end();
    });
    boxes->erase(p, boxes->end());

    return result;
  }

  int cnt;

  bool solution_found;
  vector<int> concrete_xs;
  vector<int> concrete_ys;
  vector<int> box_indices;

  void maximize_diff(Shortest &s, int var1, int var2) {
    assert(!s.empty);
    int d = s.get_diff(var1, var2);
    assert(d < INF);
    s.add_constraint(var2, var1, -d);
  }

  void rec(SearchState ss) {
    if (solution_found)
      return;
    cnt++;
    if (find(ss.concrete.begin(), ss.concrete.end(), false) == ss.concrete.end()) {
      // cerr << "FOUND" << endl;
      box_indices.clear();
      for (auto cs : ss.candidates) {
        // cerr << cs.size() << " " << cs << endl;
        assert(cs.size() == 1);
        box_indices.push_back(cs.front().index);
      }

      // make sure that box occupies as much space as possible
      maximize_diff(xs, symbolic_bps[0].bottom_left.X, x0);
      maximize_diff(ys, symbolic_bps[0].bottom_left.Y, y0);
      maximize_diff(xs, symbolic_bps[1].bottom_left.X, x0);
      maximize_diff(ys, symbolic_bps[1].bottom_left.Y, y0);

      concrete_xs = xs.any_solution();
      concrete_ys = ys.any_solution();
      solution_found = true;
      return;
    }

    int idx = -1;
    for (int i = 0; i < ss.candidates.size(); i++)
      if (!ss.concrete[i] &&
          (idx == -1 || ss.candidates[idx].size() > ss.candidates[i].size()))
        idx = i;
    // cerr << "Picking spb " << idx << " with "
    //      << ss.candidates[idx].size() << " candidates" << endl;

    auto &sbp = symbolic_bps[idx];
    for (auto cand : ss.candidates[idx]) {
      //cerr << "trying candidate " << cand << endl;
      auto ss2 = ss;
      ss2.concrete[idx] = true;
      xs.push();
      ys.push();

      // make concrete
      xs.add_constraint(sbp.top_right.X, sbp.bottom_left.Y, cand.a);
      xs.add_constraint(sbp.bottom_left.X, sbp.top_right.Y, -cand.a);
      ys.add_constraint(sbp.top_right.X, sbp.bottom_left.Y, cand.b);
      ys.add_constraint(sbp.bottom_left.X, sbp.top_right.Y, -cand.b);
      if (xs.empty || ys.empty) {
        xs.pop();
        ys.pop();
        continue;
      }

      ss2.taken_boxes[cand.index] = true;
      ss2.candidates[idx] = {cand};

      if (strengthen_constraints(ss2))
        rec(ss2);

      xs.pop();
      ys.pop();
    }
  }
};


Packer make_grid_packer(Coord size, int H, int W) {
  Packer p;

  p.size = size;
  p.x0 = p.xs.add_var("x0");
  p.y0 = p.ys.add_var("y0");

  auto right = p.make_symbolic_placement("right_");
  auto top = p.make_symbolic_placement("top_");


  p.touches_bottom(right);
  p.doesnt_touch_left(right);
  p.touches_left(top);
  p.doesnt_touch_bottom(top);

  //p.h_touch(top, right);
  p.v_touch(right, top);

  //const int H = 5;
  //const int W = 5;

  map<int, BoxPlacement> q;

  p.symbolic_bps = {top, right};
  for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
      if ((i + j) % 2 == 0)
        continue;
      q[j * 10 + i] = p.make_symbolic_placement(
          string("inner") + to_string(j * 10 + i));
      p.symbolic_bps.push_back(q[j * 10 + i]);
    }

  for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++) {
      if ((i + j) % 2 == 0)
        continue;
      int idx = 10 * j + i;
      if (i == 0) {
        p.touches_left(q[idx]);
      } else if (i == 1) {
        p.doesnt_touch_left(q[idx]);
      } else {
        p.h_space(q[idx - 2], q[idx]);
      }
      if (i == W - 1) {
        p.h_touch(q[idx], right);
      } else if (i == W - 2) {
        p.h_space(q[idx], right);
      }

      if (j == 0) {
        p.touches_bottom(q[idx]);
      } else if (j == 1) {
        p.doesnt_touch_bottom(q[idx]);
      } else {
        p.v_space(q[idx - 20], q[idx]);
      }
      if (j == H - 1) {
        p.v_touch(q[idx], top);
      } else if (j == H - 2) {
        p.v_space(q[idx], top);
      }

      if (i > 0 && j > 0) {
        p.h_touch(q[idx - 11], q[idx]);
      }
      if (i > 0 && j < H - 1) {
        p.v_touch(q[idx], q[idx + 9]);
      }
    }

  return p;
}

vector<Packer> make_packers(Coord size) {
  vector<Packer> result;

  result.push_back(make_grid_packer(size, 1, 1));
  result.push_back(make_grid_packer(size, 1, 3));
  result.push_back(make_grid_packer(size, 3, 1));

  Packer p;

  p.size = size;
  p.x0 = p.xs.add_var("x0");
  p.y0 = p.ys.add_var("y0");

  auto right_border = p.make_symbolic_placement("right_border_");
  auto top_border = p.make_symbolic_placement("top_border_");
  auto mid = p.make_symbolic_placement("mid_");
  auto a = p.make_symbolic_placement("a_");
  auto b = p.make_symbolic_placement("b_");

  p.touches_bottom(right_border);
  p.doesnt_touch_left(right_border);
  p.touches_left(top_border);
  p.doesnt_touch_bottom(top_border);

  p.h_touch(top_border, right_border);

  p.symbolic_bps = {top_border, right_border};
  //result.push_back(p);

  p.touches_bottom(mid);
  p.doesnt_touch_left(mid);
  p.v_touch(mid, top_border);
  p.h_space(mid, right_border);
  p.symbolic_bps.push_back(mid);

  p.touches_left(a);
  p.doesnt_touch_bottom(a);
  p.h_touch(a, mid);
  p.v_space(a, top_border);
  p.symbolic_bps.push_back(a);
  result.push_back(p);

  auto p2 = p;
  p2.touches_bottom(b);
  p2.doesnt_touch_left(b);
  p2.v_touch(b, a);
  p2.h_space(b, mid);
  p2.symbolic_bps.push_back(b);
  result.push_back(p2);

  /*p2 = p;
  p2.h_touch(mid, b);
  p2.h_touch(b, right_border);
  p2.doesnt_touch_bottom(b);
  p2.v_space(b, top_border);
  p2.symbolic_bps.push_back(b);
  result.push_back(p2);*/

  result.push_back(make_grid_packer(size, 3, 3));
  result.push_back(make_grid_packer(size, 3, 5));
  result.push_back(make_grid_packer(size, 5, 3));
  //result.push_back(make_grid_packer(size, 5, 5));

  reverse(result.begin(), result.end());
  return result;
}
