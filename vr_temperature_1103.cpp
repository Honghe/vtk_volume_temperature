//
// Created by honhe on 10/27/16.
//

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/format.hpp>
#include "boost/multi_array.hpp"
#include "MyDirector.h"

using namespace std;
using namespace boost::filesystem;

int main() {
    MyDirector *director = new MyDirector();
    director->init();
    director->startInteractor();
    delete director;
}
