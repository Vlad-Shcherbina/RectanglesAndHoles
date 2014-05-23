#include <vector>
#include <iostream>
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


bool place_in_corner(vector<BoxPlacement> &bps, vector<Box> &boxes) {
  if (boxes.size() < 2)
    return false;
  auto lc = find_largest_corner(bps);
  cerr << "largest corner: " << lc << endl;

  for (int i = 0; i < boxes.size(); i++) {
    if (boxes[i].b <= lc.second.Y - lc.first.Y) {
      int j = i ? 0 : 1;
      bps.push_back(BoxPlacement::from_box(
          boxes[i], {min(lc.second.X, lc.first.X + boxes[j].a), lc.first.Y}));
      bps.push_back(BoxPlacement::from_box(
          boxes[j], {lc.first.X, lc.first.Y + boxes[i].b}));
      assert(i != j);
      if (i < j)
        swap(i, j);
      boxes.erase(boxes.begin() + i);
      boxes.erase(boxes.begin() + j);
      return true;
    }
  }

  return false;
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

    int k = 45 * A.size() / 100;
    vector<Box> for_circle(sorted_boxes.begin(), sorted_boxes.begin() + k);
    sorted_boxes.erase(sorted_boxes.begin(), sorted_boxes.begin() + k);

    solution.push_back(make_circle(for_circle));

    while (place_in_corner(solution.back(), sorted_boxes)) {}

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
