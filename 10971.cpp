#include <iostream>
#include <algorithm>
using namespace std;

int n, ans = 1e9;
int graph[10][10];
int arr[10];
bool visited[10];

void tracking(int start, int cnt) {
    if (cnt == n - 1) {
        if (!graph[start][0]) return;

        int sum = graph[start][0];
        for (int i = 0; i < cnt; i++)
            sum += arr[i];

        ans = min(ans, sum);
        return;
    }

    for (int i = 0; i < n; i++) {
        if (graph[start][i] && !visited[i]) {
            arr[cnt] = graph[start][i];
            visited[start] = true;
            tracking(i, cnt + 1);
            visited[start] = false;
        }
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);

    cin >> n;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cin >> graph[i][j];
        }
    }

    tracking(0, 0);

    cout << ans;

    return 0;
}