typedef pair<int, int> Coord;
#define X first
#define Y second


struct Box {
  int index;
  int a;
  int b;
  int diag2() const {
    return a * a + b * b;
  }
};

ostream& operator<<(ostream &out, const Box &b) {
  out << "Box(" << b.index << ", " << b.a << ", " << b.b << ")";
  return out;
}

struct BoxPlacement {
  int index;
  Coord bottom_left, top_right;
  static BoxPlacement from_box(Box box, Coord bottom_left) {
    BoxPlacement result;
    result.bottom_left = bottom_left;
    result.top_right = Coord(bottom_left.X + box.a, bottom_left.Y + box.b);
    result.index = box.index;
    return result;
  }

  bool intersect(Coord c1, Coord c2) const {
    return max(bottom_left.X, c1.X) < min(top_right.X, c2.X) &&
           max(bottom_left.Y, c1.Y) < min(top_right.Y, c2.Y);
  }
};


pair<Coord, Coord> placements_bb(vector<BoxPlacement> bps) {
  Coord c1(INF, INF);
  Coord c2(-INF, -INF);
  for (auto bp : bps) {
    c1.X = min(c1.X, bp.bottom_left.X);
    c1.Y = min(c1.Y, bp.bottom_left.Y);
    c2.X = max(c2.X, bp.top_right.X);
    c2.Y = max(c2.Y, bp.top_right.Y);
  }
  return {c1, c2};
}

vector<BoxPlacement> repack_placements(vector<vector<BoxPlacement>> bpss) {
  int size = -INF;
  for (auto bps : bpss) {
    auto bb = placements_bb(bps);
    size = max(size, max(bb.second.X - bb.first.X, bb.second.Y - bb.first.Y));
  }
  int cols = int(sqrt(bpss.size()) + 0.9999);
  int idx = 0;
  vector<BoxPlacement> result;

  int h = 0;
  int qx = 0;
  int qy = 0;

  for (auto bps : bpss) {
    if (idx % cols == 0) {
      qy += h;
      qx = 0;
      h = 0;
    }
    auto bb = placements_bb(bps);
    for (auto bp : bps) {
      bp.bottom_left.X += qx - bb.first.X;
      bp.bottom_left.Y += qy - bb.first.Y;
      bp.top_right.X += qx - bb.first.X;
      bp.top_right.Y += qy - bb.first.Y;
      result.push_back(bp);
    }
    qx += bb.second.X - bb.first.X;
    h = max(h, bb.second.Y - bb.first.Y);
    idx++;
  }
  return result;
}


void transform(
  vector<BoxPlacement> &bps,
  int dx, int dy, bool mirror_x, bool mirror_y, bool mirror_xy) {
  for (auto &bp : bps) {
    for (auto p : {&bp.bottom_left, &bp.top_right}) {
      if (mirror_x)
        p->X *= -1;
      if (mirror_y)
        p->Y *= -1;
      if (mirror_xy)
        swap(p->X, p->Y);
      p->X += dx;
      p->Y += dy;
    }
    Coord bottom_left(
        min(bp.bottom_left.X, bp.top_right.X),
        min(bp.bottom_left.Y, bp.top_right.Y));
    Coord top_right(
        max(bp.bottom_left.X, bp.top_right.X),
        max(bp.bottom_left.Y, bp.top_right.Y));
    bp.bottom_left = bottom_left;
    bp.top_right = top_right;
  }
}


int find_intersection(
    const vector<BoxPlacement> &bps, Coord bottom_left, Coord top_right) {
  auto p = find_if(bps.begin(), bps.end(),
      [&](BoxPlacement bp) { return bp.intersect(bottom_left, top_right); });
  if (p == bps.end())
    return -1;
  return p - bps.begin();
}


pair<Coord, Coord> find_largest_corner(const vector<BoxPlacement> &bps) {
  set<Coord> candidates;
  for (auto bp : bps) {
    candidates.insert({bp.bottom_left.X, bp.top_right.Y});
    candidates.insert({bp.top_right.X, bp.bottom_left.Y});
  }
  int best_area = -1;
  pair<Coord, Coord> best = {{0, 0}, {0, 0}};
  for (auto cand : candidates) {
    if (find_intersection(bps, cand, Coord(cand.X + 2000, cand.Y + 2000)) != -1)
      continue;
    if (find_intersection(bps, cand, Coord(cand.X + 1, cand.Y + 2000000)) != -1 &&
        find_intersection(bps, cand, Coord(cand.X + 2000000, cand.Y + 1)) != -1)
      continue;
    int i1 = find_intersection(
        bps, Coord(cand.X, cand.Y - 1), Coord(cand.X + 1, cand.Y));
    if (i1 == -1)
      continue;
    int i2 = find_intersection(
        bps, Coord(cand.X - 1, cand.Y), Coord(cand.X, cand.Y + 1));
    if (i2 == -1)
      continue;
    pair<Coord, Coord> cur = {cand, {bps[i1].top_right.X, bps[i2].top_right.Y}};
    int area = (cur.second.X - cur.first.X) * (cur.second.Y - cur.first.Y);
    if (area > best_area) {
      best_area = area;
      best = cur;
    }
  }
  return best;
}
