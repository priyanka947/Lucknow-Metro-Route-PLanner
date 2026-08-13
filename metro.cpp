#include <bits/stdc++.h>
#include <fstream>
#include <limits> // Required for std::numeric_limits and LLONG_MAX
#include <queue>    // Required for std::priority_queue
#include <stack>    // Required for std::stack

#define ll long long
#define pb push_back
#define fi first
#define se second
#define mp make_pair
using namespace std;

// Global declarations
map<string, ll> M; // Maps station name to its index (Node ID)
char color[200][200] = {'\0'}; // Stores the line color between stations

// Comparator class for the Min Heap in Dijkstra's
class comparedis
{
public:
    bool operator()(pair<ll, ll> &p, pair<ll, ll> &q)
    {
        return (p.se > q.se); // For min heap (Dijkstra)
    }
};

vector<pair<ll, ll>> v[100010]; // Adjacency list: v[u] = { {v1, w1}, {v2, w2}, ... }
ll N; // Total number of stations (vertices)
string station[200]; // Maps index (Node ID) back to station name
map<string, string> tourm; // Maps tourist place to nearest metro station

// Function to handle smart card recharge
void recharge()
{
    fstream f_in, f_out;
    ll c_id, amount;
    ll current_id, current_amount;
    bool found = false;

    // 1. Get input
    cout << endl;
    cout << "Enter Card Id\n";
    cin >> c_id;
    cout << "Enter Amount\n";
    cin >> amount;

    f_in.open("paisa.txt", ios::in);
    f_out.open("temp.txt", ios::out);

    if (!f_in)
    {
        cout << "Not Found (paisa.txt missing). Cannot recharge." << endl;
        return;
    }

    // 2. Read, Find, and Write to temp.txt
    while (f_in >> current_id)
    {
        if (!(f_in >> current_amount)) break; // Safety check

        if (current_id == c_id)
        {
            ll new_balance = current_amount + amount;
            
            // Write updated balance to temp file
            f_out << current_id << "\n" << new_balance << "\n";
            
            cout << "Recharge Details\n";
            cout << "\nCard Id: " << current_id << endl;
            cout << "Initial Balance: " << current_amount << endl;
            cout << "Recharge Amount: " << amount << endl;
            cout << "Total Balance: " << new_balance << endl;
            
            found = true;
        }
        else
        {
            // Write unchanged data to temp file
            f_out << current_id << "\n" << current_amount << "\n";
        }
    }

    f_in.close();
    f_out.close();

    // 3. Replace original file with temp file
    if (found)
    {
        // On Windows/Linux, rename/replace the file.
        remove("paisa.txt");       // Delete the old file
        rename("temp.txt", "paisa.txt"); // Rename temp to original
    }
    else
    {
        cout << "Card ID " << c_id << " not found." << endl;
        remove("temp.txt"); // Delete temp file if Card ID was not found
    }
}

// Function to load tourist places data
void gettour()
{
    ifstream fin;
    string s1, s2;
    fin.open("touristplaces.txt", ios::in);
    if (!fin)
    {
        cout << "Not Found (touristplaces.txt missing)\n";
        return;
    }
    fin.seekg(0);
    fin.clear();

    // Read lines until EOF, pairing s1 (place) and s2 (station)
    while (getline(fin, s1) && getline(fin, s2))
    {
        tourm[s1] = s2;
    }
    fin.close();
}

// Function to display the Lucknow Metro Map (System Call)
void displayMap()
{
    // FIX: Uses the correct PNG file name
    const char *mapImagePath = "lucknow-metro-rail-map.png";

    // Use system call to open the image. 'start' is for Windows.
    // NOTE: If using Linux/Mac, you must change 'start' to 'xdg-open' or 'open'
    system(("start " + string(mapImagePath)).c_str());
    cout << "\nAttempting to open map file: " << mapImagePath << endl;
    
}

// Function to print the path from source to destination
void disp(ll src, ll dest, ll par[])
{
    ll i, x, y, cn = 0, ci = 0;
    stack<ll> st;
    st.push(dest);
    i = dest;
    while (par[i] != -1)
    {
        i = par[i];
        st.push(i);
    }
    char col = '\0';
    cout << "\n--- Route Path ---\n";
    while (!st.empty())
    {
        x = st.top();
        st.pop();

        // If stack is not empty, get the next station (y)
        if (!st.empty())
            y = st.top();

        cout << station[x] << " ";
        cn++;

        // This logic needs to be run *before* the last station is printed
        if (!st.empty())
        {
            if (col == '\0')
                col = color[x][y];
            else if (col != '\0' && col != color[x][y])
            {
                char c = color[x][y];
                ci++;
                // Only Red line exists currently
                if (c == 'r')
                    cout << "\t\tChange to red line";
                col = c;
            }
        }

        cout << endl;
    }
    cout << "\nNo of stations: " << cn << endl;
    cout << "No of interchange stations: " << ci << endl; // Interchange count is correct without ci-1 for this logic
    cout << "------------------\n";
}

// BFS to find the shortest path in terms of number of stops
void bfs(ll src, ll dest)
{
    bool vis[100010] = {false};
    ll par[100010];
    for (ll i = 0; i < N; i++)
        par[i] = -1;
    queue<ll> q;
    q.push(src);
    vis[src] = true;
    while (!q.empty())
    {
        ll x = q.front();
        q.pop();
        ll vsz = v[x].size();
        for (ll i = 0; i < vsz; i++)
        {
            ll y = v[x][i].fi;
            if (!vis[y])
            {
                par[y] = x;
                vis[y] = true;
                q.push(y);
            }
        }
    }
    disp(src, dest, par);
}

// Dijkstra's Algorithm to find the most economical path (based on weight/fare)
void dijkstra(ll src, ll dest)
{
    bool vis[100010] = {false};
    ll dist[100010], par[100010];
    for (ll i = 0; i < N; i++)
    {
        dist[i] = LLONG_MAX;
        par[i] = -1;
    }

    // Min Heap for Dijkstra's
    priority_queue<pair<ll, ll>, vector<pair<ll, ll>>, comparedis> pq;
    pq.push(mp(src, 0));
    dist[src] = 0;
    par[src] = -1;

    while (!pq.empty())
    {
        pair<ll, ll> k = pq.top();
        pq.pop();
        ll x = k.fi;

        // If the extracted distance is already greater than a recorded distance, skip
        if (k.se > dist[x])
            continue;

        ll vsz = v[x].size();
        for (ll i = 0; i < vsz; i++)
        {
            ll y = v[x][i].fi;
            ll w = v[x][i].se;

            // Relaxation step
            if (dist[x] != LLONG_MAX && dist[x] + w < dist[y])
            {
                par[y] = x;
                dist[y] = dist[x] + w;
                pq.push(mp(y, dist[y])); // Push updated distance to priority queue
            }
        }
    }

    if (dist[dest] == LLONG_MAX)
    {
        cout << "\nDestination not reachable from source.\n";
    }
    else
    {
        cout << "\nTotal Cost (in units): " << dist[dest] << endl;
        disp(src, dest, par);
    }
}

// Function to map station names to indices
void consmap()
{
    ifstream fin;
    string s;
    fin.open("list.txt", ios::in);
    ll l = 0;
    fin.seekg(0);
    fin.clear();
    while (getline(fin, s))
    {
        M[s] = l;
        station[l] = s;
        l++;
    }
    N = l; // Set the number of vertices (count of unique stations)
    fin.close();
}

// Function to add edges from a line file
void addedge(const char fname[], ll w)
{
    ifstream fin;
    string s;
    ll x, y;
    fin.open(fname, ios::in);
    if (!fin)
        return; // Skip if file not found

    fin.seekg(0);
    fin.clear();

    // Read the first station
    if (!getline(fin, s))
        return;
    x = M[s];
    char c = fname[0]; // Line color is determined by the first letter of the filename

    // Read the rest of the stations
    while (getline(fin, s))
    {
        y = M[s];
        v[x].pb(mp(y, w));
        v[y].pb(mp(x, w));
        color[x][y] = c;
        color[y][x] = c;
        x = y;
    }
    fin.close();
}

// Function to construct the graph (load all lines)
void consgraph()
{
    // Clear the adjacency list before rebuilding the graph
    for (int i = 0; i < N; ++i)
    {
        v[i].clear();
    }

    // Load Lucknow Metro Red Line (Weight 1 per hop)
    addedge("redline.txt", 1);
}

int main()
{
    string source, destination;
    ll src, dest, choice, dec;
    char ch;

    // 1. Load Data
    gettour(); // Load tourist places
    consmap(); // Map station names to indices

    cout << "DEBUG: Data Loading Complete. Starting Main Menu." << endl; // Debug flag

    do
    {
        // Clear cin failure flags and ignore rest of line after previous cin>>ch or cin>>dec
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        cout << endl;
        cout << "===== Lucknow Metro Route Planner =====" << endl;
        cout << "1. Route between two stations\n";
        cout << "2. Check nearest metro station to a tourist place\n";
        cout << "3. Recharge your Smart Card\n";
        cout << "4. See the Metro Map\n";
        cout << "5. Exit\n"; // Added explicit exit option
        cout << "---------------------------------------" << endl;
        cout << "Enter choice: ";

        // Attempt to read choice, breaking the loop if input fails (e.g., user hits Ctrl+D/Z)
        if (!(cin >> dec))
        {
            break;
        }

        switch (dec)
        {
        case 1:
            do
            {
                // Clear buffer after reading 'dec' or 'choice'
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                consgraph(); // Rebuild graph

                cout << "Enter Source Station (e.g., Charbagh Railway Station): ";
                getline(cin, source);

                cout << "Enter Destination Station (e.g., Munshipulia): ";
                getline(cin, destination);

                if (M.find(source) == M.end() || M.find(destination) == M.end())
                {
                    cout << "Error: One or both stations not found. Check spelling.\n";
                    break;
                }

                src = M[source];
                dest = M[destination];

                cout << "\nPathfinding Options:" << endl;
                cout << "1. For most economic path (Min Cost/Hops)\n";
                cout << "2. For shortest path (Min Hops)\n";
                cout << "Choice: ";

                if (!(cin >> choice))
                {
                    cout << "Input error for choice. Returning to main menu.\n";
                    break;
                }

                switch (choice)
                {
                case 1:
                    dijkstra(src, dest);
                    break;
                case 2:
                    bfs(src, dest);
                    break;
                default:
                    cout << "Invalid choice.\n";
                }

                cout << "\nDo you wish to check for any other route (Y/N)? ";
                cin >> ch;
            } while (ch == 'Y' || ch == 'y');
            break;
        case 2:
            do
            {
                string place;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                cout << "Enter a place (e.g., Bara Imambara): ";
                getline(cin, place);

                // Check if the key exists before accessing
                if (tourm.count(place))
                {
                    cout << "\nNearest Metro Station: " << tourm.at(place) << endl;
                }
                else
                {
                    cout << "No nearby station found for this place in the database.\n";
                }

                cout << "\nDo you wish to check for any other place (Y/N)? ";
                cin >> ch;
            } while (ch == 'Y' || ch == 'y');
            break;
        case 3:
            do
            {
                recharge();
                cout << "\nDo you wish to recharge some other smart card (Y/N)? ";
                cin >> ch;
            } while (ch == 'Y' || ch == 'y');
            break;

        case 4:
            displayMap();
            break;
        case 5: // Exit option
            ch = 'N';
            break;
        default:
            cout << "Invalid main menu choice.\n";
        }

        // If the user chose to exit (case 5), 'ch' is already set to 'N'
        if (dec != 5)
        {
            cout << "\nDo you wish to go back to main menu (Y/N)? ";
            if (!(cin >> ch))
                break; // Break if input fails
        }
        else
        {
            break;
        }

    } while (ch == 'Y' || ch == 'y');

    cout << "Program exited successfully." << endl;
    return 0;
}