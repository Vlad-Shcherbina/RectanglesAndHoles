#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstddef>
#include <cstdlib>

#include "shortest.h"


using namespace std;


#include "box.h"
#include "circle.h"


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
