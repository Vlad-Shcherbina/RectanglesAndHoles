#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstddef>
#include <cstdlib>

#include "pretty_printing.h"

#include "shortest.h"


using namespace std;


#include "box.h"
#include "circle.h"
#include "pack.h"


vector<Corner> candidate_corners(
    const vector<BoxPlacement> &bps, int clearance, int limit) {
  vector<Corner> result;
  vector<BoxPlacement> bbs;

  vector<Corner> corners = find_all_corners(bps, clearance);
  sort(corners.begin(), corners.end(),
      [](const Corner &c1, const Corner &c2) {
        auto scoring_fn = [](const Corner &c) { return c.w * c.h; };
        return scoring_fn(c1) > scoring_fn(c2);
      });
  for (const auto &corner : corners) {
    bool clear = true;
    auto new_bb = corner.bounding_box(clearance, clearance);
    for (const auto &bb : bbs)
      if (new_bb.intersect(bb)) {
        clear = false;
        break;
      }
    if (clear) {
      result.push_back(corner);
      bbs.push_back(new_bb);
      if (result.size() >= limit)
        break;
    }
  }
  return result;
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
        [](Box b1, Box b2) {return b1.diag2() > b2.diag2(); });

    vector<vector<BoxPlacement> > solution;

    int k = 45 * A.size() / 100;
    vector<Box> for_circle(sorted_boxes.begin(), sorted_boxes.begin() + k);
    sorted_boxes.erase(sorted_boxes.begin(), sorted_boxes.begin() + k);

    solution.push_back(make_circle(for_circle));

    int max_size = -1;
    for (auto box : sorted_boxes) {
      max_size = max(max_size, max(box.a, box.b));
    }
    cerr << "max size " << max_size << endl;
    auto ccs = candidate_corners(solution.back(), 2 * max_size, sorted_boxes.size() / 4);
    cerr << ccs << endl;

    while (true) {
      int max_size = -1;
      for (auto box : sorted_boxes) {
        max_size = max(max_size, max(box.a, box.b));
      }

      Corner corner = find_best_corner(solution.back(), 2 * max_size);

      auto packers = make_packers(Coord(corner.w, corner.h));

      bool solved = false;
      for (auto &packer : packers) {
        packer.solve(both_orientations(sorted_boxes));
        if (packer.packings.empty())
          continue;
        auto t = packer.packings.front().bps;
        cerr << t.size() << " blocks" << endl;
        transform(t, corner.origin.X, corner.origin.Y, false, false, false);
        transform(t, 0, 0, corner.flip_x, corner.flip_y, false);
        copy(t.begin(), t.end(), back_inserter(solution.back()));
        remove_used_boxes(t, sorted_boxes);
        solved = true;
        break;
      }
      if (!solved)
        break;
    }

    for (auto box : sorted_boxes) {
      solution.emplace_back();
      solution.back().push_back(BoxPlacement::from_box(box, {0, 0}));
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
  while (true) {
    int n;
    cin >> n;
    if (cin.eof())
      break;
    vector<int> A(n), B(n);
    for (int i = 0; i < n; i++)
      cin >> A[i];
    for (int i = 0; i < n; i++)
      cin >> B[i];

    if (false) {
      ofstream input_dump("input_dump.txt");
      input_dump << n << endl;
      for (int x : A)
        input_dump << x << endl;
      for (int x : B)
        input_dump << x << endl;
    }

    auto result = RectanglesAndHoles().place(A, B);
    cerr.flush();
    // to ensure that java runner picks up stderr
    this_thread::sleep_for(chrono::milliseconds(200));

    for (int r : result)
      cout << r << endl;
    cout.flush();
  }
  return 0;
}
#endif
