#include <algorithm>

using namespace std;

// The 3rd parameter for both functions is a "hint" about where to start. The answer will be the same no matter what the hint is


// comp(e,v)  iff e <  v
// !comp(e,v) iff e >= v 
// returns iterator to the least element e such that comp(e,v) is false
template<class RandomAccessIterator, class T, class Compare>
RandomAccessIterator exponentialSearchLowerBound(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator e, const T& v, Compare comp)
{
    RandomAccessIterator lowerBound = first;
    RandomAccessIterator upperBound = last;
    // lowerBound is inclusive, upperBound is exclusive.
    bool c = comp(*e, v);
    if (!c) {
        // e >= v, so exponential search to the left
        int gap = 1;
        while (!c) {
            upperBound = e + 1;
            e -= gap;
            if (e < lowerBound) {
                e = lowerBound - 1;
                break;
            }
            gap *= 2;
            c = comp(*e, v);
        }
        // at this point, either e == lowerBound, or e < v
        lowerBound = e + 1;
    } else {
        // e < v, so exponential search to the right
        int gap = 1;
        while (c) {
            lowerBound = e + 1;
            e += gap;
            if (e >= upperBound) {
                e = upperBound - 1;
                break;
            }
            gap *= 2;
            c = comp(*e, v);
        }
        // at this point, either e == upperBound - 1, or e >= v
        upperBound = e + 1;
    }
    // lower_bound does binary search in [lowerBound, upperBound)
    auto ret = lower_bound(lowerBound, upperBound, v, comp);
    if (ret == upperBound) {
        return last;
    } else {
        return ret;
    }
}

// comp(e,v)  iff e <  v
// !comp(e,v) iff e >= v
// comp(v,e)  iff v <  e
// !comp(v,e) iff v >= e
// returns iterator to the least element e such that comp(v,e) is true (v < e)
template<class RandomAccessIterator, class T, class Compare>
RandomAccessIterator exponentialSearchUpperBound(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator e, const T& v, Compare comp)
{
    RandomAccessIterator lowerBound = first;
    RandomAccessIterator upperBound = last;
    // lowerBound is inclusive, upperBound is exclusive.
    bool c = comp(v, *e);
    if (c) {
        // v < e, so exponential search to the left
        int gap = 1;
        while (c) {
            upperBound = e + 1;
            e -= gap;
            if (e < lowerBound) {
                e = lowerBound - 1;
                break;
            }
            gap *= 2;
            c = comp(v, *e);
        }
        // at this point, either e == lowerBound - 1, or v >= e
        lowerBound = e + 1;
    } else {
        // v >= e, so exponential search to the right
        int gap = 1;
        while (!c) {
            lowerBound = e + 1;
            e += gap;
            if (e >= upperBound) {
                e = upperBound - 1;
                break;
            }
            gap *= 2;
            c = comp(v, *e);
        }
        // at this point, either e == upperBound - 1, or v < e
        upperBound = e + 1;
    }
    // upper_bound does binary search in [lowerBound, upperBound)
    auto ret = upper_bound(lowerBound, upperBound, v, comp);
    if (ret == upperBound) {
        return last;
    } else {
        return ret;
    }
}
