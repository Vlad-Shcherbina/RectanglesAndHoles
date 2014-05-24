class Arc {
public:
  vector<Box> boxes;
  int delta_x, delta_y;
  int64_t full_area;
  int64_t area;

  Arc(vector<Box> boxes) : boxes(boxes) {
    precompute();
  }

  void precompute() {
    sort(boxes.begin(), boxes.end(),
        [](Box b1, Box b2){ return b1.b * b2.a > b1.a * b2.b; });
    delta_x = 0;
    delta_y = 0;
    full_area = 0;
    for (auto box : boxes) {
      full_area += int64_t(box.a) * int64_t(delta_y);
      delta_x += box.a;
      delta_y += box.b;
    }
  }

  vector<BoxPlacement> place(int x_penalty = 0) {
    int pdx = delta_x - x_penalty;
    vector<BoxPlacement> result;
    int x = 0;
    int y = 0;
    area = 0;
    for (int i = 0; i < boxes.size(); i++) {
      Box box = boxes[i];
      int x_offset = 0;
      if (i > 0 && x_penalty > 0) {
        Box prev_box = boxes[i - 1];
        x_offset = min(x_penalty, box.a + prev_box.a);
        x_penalty -= x_offset;
      }
      result.push_back(BoxPlacement::from_box(box, Coord(x - x_offset, y)));
      x += box.a - x_offset;
      y += box.b;
      area += (int64_t)box.b * (int64_t)(pdx - x);
    }
    return result;
  }
};


pair<int64_t, vector<BoxPlacement>> make_random_circle(const vector<Box> &boxes) {
  while (true) {
    vector<Box> arc_boxes[4];
    for (auto box : boxes) {
      if (rand() % 2)
        swap(box.a, box.b);
      arc_boxes[rand() % 4].push_back(box);
    }
    vector<BoxPlacement> result;
    if (arc_boxes[0].empty())
      continue;
    if (arc_boxes[1].empty())
      continue;
    if (arc_boxes[2].empty())
      continue;
    if (arc_boxes[3].empty())
      continue;
    Arc arc[] = {arc_boxes[0], arc_boxes[1], arc_boxes[2], arc_boxes[3]};

    int px = arc[0].delta_x + arc[1].delta_y - arc[2].delta_x - arc[3].delta_y;
    int py = -arc[0].delta_y + arc[1].delta_x + arc[2].delta_y - arc[3].delta_x;
    if (px < 0 || py < 0)
      continue;

    int64_t total_area = 0;
    vector<BoxPlacement> t = arc[0].place(px);
    arc[0].delta_x -= px;
    if (arc[0].delta_x < 0)
      continue;
    copy(t.begin(), t.end(), back_inserter(result));
    total_area += arc[0].area;

    t = arc[1].place(py);
    arc[1].delta_x -= py;
    if (arc[1].delta_x < 0)
      continue;
    transform(t,
        arc[0].delta_x,
        arc[0].delta_y,
        true, false, true);
    copy(t.begin(), t.end(), back_inserter(result));
    total_area += arc[1].area;

    t = arc[2].place();
    transform(t,
        arc[0].delta_x + arc[1].delta_y,
        arc[0].delta_y - arc[1].delta_x,
        true, true, false);
    copy(t.begin(), t.end(), back_inserter(result));
    total_area += arc[2].area;

    t = arc[3].place();
    transform(t,
        arc[0].delta_x + arc[1].delta_y - arc[2].delta_x,
        arc[0].delta_y - arc[1].delta_x - arc[2].delta_y,
        false, true, true);
    copy(t.begin(), t.end(), back_inserter(result));
    total_area += arc[3].area;

    total_area += (int64_t)(arc[0].delta_x - arc[3].delta_y) * (arc[1].delta_x - arc[0].delta_y);

    //cerr << "total area: " << total_area << endl;

    return {total_area, result};
  }
}


vector<BoxPlacement> make_circle(vector<Box> boxes) {
  pair<int64_t, vector<BoxPlacement>> best;
  best.first = -1;
  int n = boxes.size() < 200 ? 5000 : 1000;
  for (int i = 0; i < n; i++) {
    auto cur = make_random_circle(boxes);
    if (cur.first > best.first) {
      best = cur;
    }
  }
  return best.second;
}
