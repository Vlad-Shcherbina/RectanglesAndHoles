typedef pair<int, int> Coord;
#define X first
#define Y second


struct Box {
  int index;
  int a;
  int b;
};


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
