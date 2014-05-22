#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstddef>

#include "shortest.h"


using namespace std;


struct Box {
  int index;
  int a;
  int b;
};


typedef pair<int, int> Coord;
#define X first
#define Y second


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


class Arc {
public:
  vector<Box> boxes;
  int delta_x, delta_y;
  int64_t full_area;

  Arc(vector<Box> boxes) : boxes(boxes) {
    // TODO: randomize a and b
    precompute();
  }

  void precompute() {
    for (auto &box : boxes)
      if (rand() % 2)
        swap(box.a, box.b);
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
    vector<BoxPlacement> result;
    int x = 0;
    int y = 0;
    //for (auto box : boxes) {
    for (int i = 0; i < boxes.size(); i++) {
      Box box = boxes[i];
      int x_offset = 0;
      if (i > 0 && x_penalty > 0) {
        Box prev_box = boxes[i - 1];
        x_offset = min(x_penalty, box.a + prev_box.a);
        x_penalty -= x_offset;
      }
      //full_area += box.a * delta_y;
      result.push_back(BoxPlacement::from_box(box, Coord(x - x_offset, y)));
      x += box.a - x_offset;
      y += box.b;
    }
    return result;
  }
};


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


vector<BoxPlacement> make_circle(vector<Box> boxes) {
  while (true) {
    vector<Box> arc_boxes[4];
    for (auto box : boxes)
      arc_boxes[rand() % 4].push_back(box);
    vector<BoxPlacement> result;
    Arc arc[] = {arc_boxes[0], arc_boxes[1], arc_boxes[2], arc_boxes[3]};

    int px = arc[0].delta_x + arc[1].delta_y - arc[2].delta_x - arc[3].delta_y;
    int py = -arc[0].delta_y + arc[1].delta_x + arc[2].delta_y - arc[3].delta_x;
    if (px < 0 || py < 0)
      continue;

    vector<BoxPlacement> t = arc[0].place(px);
    arc[0].delta_x -= px;
    copy(t.begin(), t.end(), back_inserter(result));

    t = arc[1].place(py);
    arc[1].delta_x -= py;
    transform(t,
        arc[0].delta_x, arc[0].delta_y,
        true, false, true);
    copy(t.begin(), t.end(), back_inserter(result));

    t = arc[2].place();
    transform(t,
        arc[0].delta_x + arc[1].delta_y, arc[0].delta_y - arc[1].delta_x,
        true, true, false);
    copy(t.begin(), t.end(), back_inserter(result));

    t = arc[3].place();
    transform(t,
        arc[0].delta_x + arc[1].delta_y - arc[2].delta_x,
        arc[0].delta_y - arc[1].delta_x - arc[2].delta_y,
        false, true, true);
    copy(t.begin(), t.end(), back_inserter(result));

    return result;
  }
}


class RectanglesAndHoles {
public:
  vector<Box> boxes;
  vector<int> place(vector<int> A, vector<int> B) {
    cerr << "n = " << A.size() << endl;
    boxes.clear();
    for (int i = 0; i < A.size(); i++) {
      Box box;
      box.index = i;
      box.a = A[i], B[i];
      box.b = B[i];
      boxes.push_back(box);
    }

    vector<Box> sorted_boxes;
    for (auto box : boxes) {
      if (box.b > box.a)
        swap(box.b, box.a);
      sorted_boxes.push_back(box);
    }
    sort(sorted_boxes.begin(), sorted_boxes.end(),
        [](Box b1, Box b2) {return b1.a > b2.a; });

    vector<vector<BoxPlacement> > solution;

    int k = A.size() / 2;
    vector<Box> for_circle(sorted_boxes.begin(), sorted_boxes.begin() + k);
    sorted_boxes.erase(sorted_boxes.begin(), sorted_boxes.begin() + k);

    solution.push_back(make_circle(for_circle));

    vector<int> result;
    for (int i = 0; i + 4 <= sorted_boxes.size(); i += 4) {
      int x = 0;
      int y = 0;

      solution.emplace_back();

      auto box1 = sorted_boxes[i];
      auto box2 = sorted_boxes[i + 2];
      auto box3 = sorted_boxes[i + 1];
      auto box4 = sorted_boxes[i + 3];
      swap(box2.a, box2.b);
      swap(box4.a, box4.b);

      int w = min(box1.a, box3.a);
      int h = min(box2.b, box4.b);

      solution.back().push_back(BoxPlacement::from_box(
        box1, Coord(x, y - box1.b)));

      solution.back().push_back(BoxPlacement::from_box(
        box2, Coord(x + w, y)));

      solution.back().push_back(BoxPlacement::from_box(
        box3, Coord(x + w - box3.a, y + h)));

      solution.back().push_back(BoxPlacement::from_box(
        box4, Coord(x - box4.a, y + h - box4.b)));
    }

    for (int i = sorted_boxes.size() / 4 * 4; i < sorted_boxes.size(); i++) {
      solution.push_back({BoxPlacement::from_box(
          sorted_boxes[i], Coord(0, 0))});
    }

    return box_placements_to_result(repack_placements(solution));
  }

  vector<int> box_placements_to_result(vector<BoxPlacement> box_placements) {
    vector<BoxPlacement> q(box_placements.size());
    for (auto bp : box_placements) {
      q[bp.index] = bp;
    }
    vector<int> result;
    for (int i = 0; i < q.size(); i++) {
      assert(q[i].index == i);
      int w = q[i].top_right.X - q[i].bottom_left.X;
      int h = q[i].top_right.Y - q[i].bottom_left.Y;
      result.push_back(q[i].bottom_left.X);
      result.push_back(q[i].bottom_left.Y);
      if (w == boxes[i].a && h == boxes[i].b) {
        result.push_back(0);
      } else if (w == boxes[i].b && h == boxes[i].a) {
        result.push_back(1);
      } else {
        assert(false);
      }
    }
    return result;
  }
};


#ifndef SUBMISSION
int main(int argc, char *argv[]) {
  int n;
  cin >> n;
  vector<int> A(n), B(n);
  for (int i = 0; i < n; i++)
    cin >> A[i];
  for (int i = 0; i < n; i++)
    cin >> B[i];

  auto result = RectanglesAndHoles().place(A, B);
  cerr.flush();
  // to ensure that java runner picks up stderr
  this_thread::sleep_for(chrono::milliseconds(200));

  for (int r : result)
    cout << r << endl;
  cout.flush();
  return 0;
}
#endif
