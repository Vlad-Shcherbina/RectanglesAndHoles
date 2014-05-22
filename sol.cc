#include <vector>
#include <iostream>
#include <chrono>
#include <thread>


using namespace std;


class RectanglesAndHoles {
public:
  vector<int> place(vector<int> A, vector<int> B) {
    cerr << "n = " << A.size() << endl;
    vector<int> result;
    for (int i = 0; i < A.size(); i++) {
      int idx = i / 4;
      int x = idx % 10 * 2010;
      int y = idx / 10 * 2010;
      if (i % 4 == 0) {
        result.push_back(x);
        result.push_back(y - B[i]);
        result.push_back(0);
      } else if (i % 4 == 1) {
        result.push_back(x + 1);
        result.push_back(y);
        result.push_back(0);
      } else if (i % 4 == 2) {
        result.push_back(x + 1 - A[i]);
        result.push_back(y + 1);
        result.push_back(0);
      } else {
        result.push_back(x - A[i]);
        result.push_back(y + 1 - B[i]);
        result.push_back(0);
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
