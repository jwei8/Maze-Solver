#include "treasureMap.h"
#include "queue.h"
using namespace std;

treasureMap::treasureMap(const PNG &baseim, const PNG &mazeim, pair<int, int> s)
{

    base = baseim;
    maze = mazeim;
    start = s;
}

void treasureMap::setGrey(PNG &im, pair<int, int> loc)
{

    RGBAPixel *pixel = im.getPixel(loc.first, loc.second);
    pixel->r = 2 * (pixel->r / 4);
    pixel->g = 2 * (pixel->g / 4);
    pixel->b = 2 * (pixel->b / 4);
}

void treasureMap::setLOB(PNG &im, pair<int, int> loc, int d)
{

    RGBAPixel *pixel = im.getPixel(loc.first, loc.second);
    pixel->r = (pixel->r & 0b11111100) | ((d & 0b00110000) >> 4);
    pixel->g = (pixel->g & 0b11111100) | ((d & 0b00001100) >> 2);
    pixel->b = (pixel->b & 0b11111100) | (d & 0b00000011);
}

PNG treasureMap::renderMap()
{

    PNG copy = base;
    vector<vector<bool>> visited;
    vector<vector<int>> length;
    Queue<pair<int, int>> list_visited;
    for (unsigned int width = 0; width < base.width(); width++)
    {
        visited.emplace_back();
        length.emplace_back();
        for (unsigned int height = 0; height < base.height(); height++)
        {
            visited.back().push_back(false);
            length.back().push_back(0);
        }
    }
    visited[start.first][start.second] = true;
    length[start.first][start.second] = 0;
    setLOB(copy, start, 0);
    list_visited.enqueue(start);

    // 4. While the queue is not empty:
    //      a. set the current location, curr, to the value at the front of the queue (and dequeue).
    //      b. for each compass neighbor, p, of curr in the order of left, below, right, above:
    //          i. if p is "good" (defined below) then
    //              1) mark p's location as visited
    //              2) set p's distance to the start to be curr's distance, + 1.
    //              3) encode the distance in the lower order bits of the treasure map
    //              4) enqueue p
    //
    // 5. return the image
    pair<int, int> curr;

    while (!list_visited.isEmpty())
    {

        curr = list_visited.dequeue();
        vector<pair<int, int>> neighbours = neighbors(curr);
        for (int i = 0; i < 4; i++)
        {
            if (good(visited, curr, neighbours[i]))
            {
                visited[neighbours[i].first][neighbours[i].second] = true;
                length[neighbours[i].first][neighbours[i].second] = length[curr.first][curr.second] + 1;
                setLOB(copy, neighbours[i], length[neighbours[i].first][neighbours[i].second]);
                list_visited.enqueue(neighbours[i]);
            }
        }
    }
    //     next.first = curr.first;
    //     next.second = curr.second + 1;
    //     if (good(visited, curr, next))
    //     {
    //         visited[curr.first][curr.second + 1] = true;
    //         length[curr.first][curr.second + 1] = length[curr.first][curr.second] + 1;
    //         setLOB(copy, next, length[curr.first][curr.second + 1]);
    //         list_visited.enqueue(next);
    //     }

    //     next.first = curr.first + 1;
    //     next.second = curr.second;
    //     if (good(visited, curr, next))
    //     {
    //         visited[curr.first + 1][curr.second] = true;
    //         length[curr.first + 1][curr.second] = length[curr.first][curr.second] + 1;
    //         setLOB(copy, next, length[curr.first + 1][curr.second]);
    //         list_visited.enqueue(next);
    //     }
    //     next.first = curr.first;
    //     next.second = curr.second - 1;
    //     if (good(visited, curr, next))
    //     {
    //         visited[curr.first][curr.second - 1] = true;
    //         length[curr.first][curr.second - 1] = length[curr.first][curr.second] + 1;
    //         setLOB(copy, next, length[curr.first][curr.second - 1]);
    //         list_visited.enqueue(next);
    //     }

    //     next.first = curr.first - 1;
    //     next.second = curr.second;
    //     if (good(visited, curr, next))
    //     {
    //         visited[curr.first - 1][curr.second] = true;
    //         length[curr.first - 1][curr.second] = length[curr.first][curr.second] + 1;
    //         setLOB(copy, next, length[curr.first - 1][curr.second]);
    //         list_visited.enqueue(next);
    //     }
    // }

    return copy;
}

//draws the entire maze on a copy of the base image by darkening the locations
// so it's visible as a shadow. This can be done by iterating over every
// pixel, but it will be faster to begin from the start position
// and find the points in the maze from there. Returns an image whose
// non-maze pixels are the same as the base image, and whose maze pixels
// have colour values that are 50% of those in the base image, for each of
// red, green and blue. (We specify the computation of the colours in the
// description of a setGrey function, below.)
//
// This function also draws a red 7px x 7px square at the start
// location. If the start location is near the edge of the image,
// just draw the existing portion of the square. The start
// should be at the center of the square.
PNG treasureMap::renderMaze()
{
    PNG copy = base;

    vector<vector<bool>> visited;

    Queue<pair<int, int>> toVisit;

    for (unsigned int width = 0; width < base.width(); width++)
    {
        visited.emplace_back();

        for (unsigned int height = 0; height < base.height(); height++)
        {
            visited.back().push_back(false);
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

            if (good(visited, curr, neighbours[i]))
            {
                visited[neighbours[i].first][neighbours[i].second] = true;
                setGrey(copy, neighbours[i]);
                toVisit.enqueue(neighbours[i]);
            }
        }
    }

    for (int width = start.first - 3; width <= start.first + 3; width++)
    {
        for (int height = start.second - 3; height <= start.second + 3; height++)
        {
            if (width >= 0 & width <= (int)base.width() & height >= 0 & height <= (int)base.height())
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

// tests next, a neighbor (adjacent vertex) of curr to see if it is
// 1. within the image, 2. unvisited, and 3. the same colour as curr in
// the maze image (member var).  An entry in table v is true if a cell
// has previously been visited, and false, otherwise. Colour sameness is
// measured using the == operator on RGBAPixel values.
bool treasureMap::good(vector<vector<bool>> &v, pair<int, int> curr, pair<int, int> next)
{
    if (next.first >= 0 & next.first < (int)base.width())
    {
        if (next.second >= 0 & next.second < (int)base.height())
        {
            if (!v[next.first][next.second])
            {
                RGBAPixel *pixel_curr = maze.getPixel(curr.first, curr.second);
                RGBAPixel *pixel_next = maze.getPixel(next.first, next.second);
                if (*pixel_curr == *pixel_next)
                {
                    return true;
                }
                return false;
            }
            return false;
        }
        return false;
    }
    return false;
}

// builds a vector containing the locations of the
// four vertices adjacent to curr:
// left, below, right, above.
// does not pay any attention to whether or not the neighbors are
// valid (in the image, previously visited, or the right colour).
vector<pair<int, int>> treasureMap::neighbors(pair<int, int> curr)
{
    vector<pair<int, int>> neighbors;

    neighbors.push_back(make_pair(curr.first, curr.second + 1));
    neighbors.push_back(make_pair(curr.first, curr.second - 1));
    neighbors.push_back(make_pair(curr.first + 1, curr.second));
    neighbors.push_back(make_pair(curr.first - 1, curr.second));
    return neighbors;

}
