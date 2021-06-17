#include "decoder.h"
#include "queue.h"
#include "stack.h"
using namespace std;

// initializes variables and build solution path as a vector
// of points that corresponds to a sequence of pixels
// beginning at pathPts[0] == start, and ending with
// pathPts[pathPts.size() - 1] == end.
//
// You will want to modify the BFS algorithm you wrote for the
// treasureMap renderer in order to solve this problem.
//
// As a hint, you will want an additional working vector whose purpose
// is to encode, for every location, the location from which it
// was first discovered. This working structure will help you
// determine the path from any location to the start (and thus
// from the start to any location).

decoder::decoder(const PNG &tm, pair<int, int> s)
    : start(s), mapImg(tm)
{

    pair<int, int> curr;
    vector<vector<pair<int, int>>> discovered;
    vector<vector<bool>> visited;
    vector<vector<int>> shortestDistance;
    int largest = 0;
    int largest_x = -1;
    int largest_y = -1;

    Queue<pair<int, int>> toVisit;
    for (unsigned int width = 0; width < mapImg.width(); width++)
    {
        visited.emplace_back();
        shortestDistance.emplace_back();
        discovered.emplace_back();

        for (unsigned int height = 0; height < mapImg.height(); height++)
        {
            visited.back().push_back(false);
            shortestDistance.back().push_back(0);
            discovered.back().push_back(make_pair(-1,-1));
        }
    }
    
    toVisit.enqueue(start);
    visited[start.first][start.second] = true;
    shortestDistance[start.first][start.second] = 0;



    while (!toVisit.isEmpty())
    {

        curr = toVisit.dequeue();
        vector<pair<int, int>> neighbours = neighbors(curr);
        for (int i = 0; i < 4; i++)
        {

            if (good(visited, shortestDistance, curr, neighbours[i]))
            {
                visited[neighbours[i].first][neighbours[i].second] = true;
                shortestDistance[neighbours[i].first][neighbours[i].second] = shortestDistance[curr.first][curr.second] + 1;
                toVisit.enqueue(neighbours[i]);
                largest_x = neighbours[i].first;
                largest_y = neighbours[i].second;
                discovered[neighbours[i].first][neighbours[i].second] = make_pair(curr.first, curr.second);
                largest = shortestDistance[curr.first][curr.second] + 1;

            }
        }
    }
    //cout<< "bfs finish" << endl;


    //cout<< "start shortest distance" << endl;

    // for (unsigned int x = 0; x < mapImg.width(); x++) {
    //     for (unsigned int y = 0; y < mapImg.height(); y++) {
    //         if (shortestDistance[x][y] >= largest) {
    //             largest = shortestDistance[x][y];
    //             //cout<< "find shortest distance" << endl;
    //             cout << largest <<endl;

    //             largest_x = x;
    //             largest_y = y;
    //         }   
    //     }
    // }

    pathPts.resize(largest + 1);
    int iterations = largest;
    pathPts[iterations] = make_pair(largest_x, largest_y);
    pair<int, int> currLoc = discovered[largest_x][largest_y];
    
    while(iterations > 0) {
        //cout << "encode loop" <<endl;
        iterations--;
        pathPts[iterations] = currLoc;
        currLoc = discovered[currLoc.first][currLoc.second];
    }


}

// draws the solution path on top of the given image. The solution
// is the shortest path to the point which is farthest from the
// start in the embedded maze. Solution path is drawn in (255,0,0).
// This function assumes that the pathPts member variable has been
// populated by the constructor.

PNG decoder::renderSolution(){

PNG copyOfMapImg = PNG(mapImg);

for (unsigned int i = 0; i < pathPts.size(); i++) {
   RGBAPixel * curr = copyOfMapImg.getPixel(pathPts[i].first, pathPts[i].second);
   curr->r = 255;
   curr->g = 0;
   curr->b = 0;
}

return copyOfMapImg;
}

// discovers and draws the entire maze on top of the given image.
// The maze is revealed by darkening each of the maze pixels: change
// each colour channel to be 50% of its treasure map value. (use the
// setGrey function from the treasureMap class.)
//
// Discovering the maze requires yet another breadth-first traversal
// of the treasureMap image. (you may be able to facilitate this
// within the constructor by adding another member variable to the
// decoder class. We leave that design decision up to you.)
//
// Note that the grey pixels in the maze rendered by this function
// may be a superset of the pixels in the original maze, because
// some neighboring pixels might have the "correct" lower
// order bits, just by happenstance.
//
// This function also draws a red 7px x 7px square at the start
// location. If the start location is near the edge of the image,
// just draw the existing portion of the square. The start
// should be at the center of the square.

PNG decoder::renderMaze()
{
    PNG copy = mapImg;

    vector<vector<bool>> visited;
    vector<vector<int>> shortestDistance;

    Queue<pair<int, int>> toVisit;
    for (unsigned int width = 0; width < mapImg.width(); width++)
    {
        visited.emplace_back();
        shortestDistance.emplace_back();

        for (unsigned int height = 0; height < mapImg.height(); height++)
        {
            visited.back().push_back(false);
            shortestDistance.back().push_back(0);
        }
    }
    pair<int, int> curr;
    toVisit.enqueue(start);
    visited[start.first][start.second] = true;

    while (!toVisit.isEmpty())
    {

        curr = toVisit.dequeue();
        vector<pair<int, int>> neighbours = neighbors(curr);
        for (int i = 0; i < 4; i++)
        {

            if (good(visited, shortestDistance, curr, neighbours[i]))
            {
                visited[neighbours[i].first][neighbours[i].second] = true;
                shortestDistance[neighbours[i].first][neighbours[i].second] = shortestDistance[curr.first][curr.second] + 1;
                toVisit.enqueue(neighbours[i]);
                setGrey(copy, neighbours[i]);
            }
        }
    }

    for (int width = start.first - 3; width <= start.first + 3; width++)
    {
        for (int height = start.second - 3; height <= start.second + 3; height++)
        {
            if (width >= 0 & width < (int)mapImg.width() & height >= 0 & height < (int)mapImg.height())
            {
                RGBAPixel *pixel = copy.getPixel(width, height);
                pixel->r = 255;
                pixel->g = 0;
                pixel->b = 0;
            }
        }
    }

    return copy;
}

// changes the pixel at position loc to consist of colour channel
// values which are 50% of their original. Note that because of
// our encoding scheme which essentially invalidates the lower
// order bits, you should compute each value as 2*(x/4), where
// x is the value of a colour channel.
void decoder::setGrey(PNG &im, pair<int, int> loc)
{

    RGBAPixel *pixel = im.getPixel(loc.first, loc.second);
    pixel->r = 2 * (pixel->r / 4);
    pixel->g = 2 * (pixel->g / 4);
    pixel->b = 2 * (pixel->b / 4);
}

pair<int, int> decoder::findSpot()
{
    
    return pathPts[pathPts.size() - 1];
}

int decoder::pathLength()
{
    return pathPts.size();
}

// tests a neighbor (adjacent vertex) to see if it is
// 1. within the image, 2. unvisited, and 3. coloured so that
// lower order bits fit the pattern prescribed by the encoder.
// An entry in table v is true if a cell has previously been
// visited. table d contains the shortest distance from each location
// back to the start. the (r,g,b) colour of position next must have lower order
// bits (XX,XX,XX) that, when interpreted as an integer between 0 and 63,
// inclusive, is d[curr.second][curr.first] + 1 (mod 64).
bool decoder::good(vector<vector<bool>> &v, vector<vector<int>> &d, pair<int, int> curr, pair<int, int> next)
{

    if (next.first >= 0 & next.first < (int)mapImg.width())
    {
        if (next.second >= 0 & next.second < (int)mapImg.height())
        {
            if (!v[next.first][next.second])
            {   RGBAPixel *pixel = mapImg.getPixel(next.first, next.second);
                int d_curr= d[curr.first][curr.second];
                if (compare(*pixel, d_curr))
                {
                    //cout<< "this reach" <<endl;
                    return true;
                }
                //cout<< "this reach" <<endl;
                return false;
            }
            return false;
        }
        return false;
    }
    return false;
}

vector<pair<int, int>> decoder::neighbors(pair<int, int> curr)
{

    vector<pair<int, int>> neighbors;

    neighbors.push_back(make_pair(curr.first - 1, curr.second));
    neighbors.push_back(make_pair(curr.first, curr.second + 1));
    neighbors.push_back(make_pair(curr.first + 1, curr.second));
    neighbors.push_back(make_pair(curr.first , curr.second - 1));
    return neighbors;
}

// tests whether p's lower order bits, when interpreted as a
// number, is d+1 (mod 64).
bool decoder::compare(RGBAPixel p, int d)
{
    int mazeValue = (p.r % 4) * 16 + (p.g % 4) * 4 + (p.b % 4);

    if (mazeValue == (d + 1) % 64)
    {
        return true;
    }
    return false;
}