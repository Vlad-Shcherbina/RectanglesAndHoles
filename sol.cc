#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

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


class RectanglesAndHoles {
public:
  vector<Box> boxes;
  vector<int> place(vector<int> A, vector<int> B) {
    boxes.clear();
    for (int i = 0; i < A.size(); i++) {
      Box box;
      box.index = i;
      box.a = A[i];
      box.b = B[i];
      boxes.push_back(box);
    }

    vector<BoxPlacement> box_placements;

    cerr << "n = " << A.size() << endl;
    vector<int> result;
    for (int i = 0; i < A.size(); i++) {
      int idx = i / 4;
      int x = idx % 10 * 2010;
      int y = idx / 10 * 2010;
      if (i % 4 == 0) {
        box_placements.push_back(BoxPlacement::from_box(
          boxes[i], Coord(x, y - B[i])));
      } else if (i % 4 == 1) {
        box_placements.push_back(BoxPlacement::from_box(
          boxes[i], Coord(x + 2, y)));
      } else if (i % 4 == 2) {
        box_placements.push_back(BoxPlacement::from_box(
          boxes[i], Coord(x + 2 - A[i], y + 2)));
      } else {
        box_placements.push_back(BoxPlacement::from_box(
          boxes[i], Coord(x - A[i], y + 2 - B[i])));
      }
    }
    return box_placements_to_result(box_placements);
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
