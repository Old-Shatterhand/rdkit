//
//  Copyright (C) 2019-2023 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include "catch.hpp"

#include <RDGeneral/test.h>
#include <iostream>
#include <fstream>
#include "Conrec.h"
#include <RDGeneral/Invariant.h>
#include <RDGeneral/RDLog.h>
#include <cmath>
#include <RDGeneral/utils.h>
#include <Geometry/point.h>

TEST_CASE("Conrec basics", "[conrec]") {
  SECTION("basics") {
    std::vector<RDGeom::Point2D> pts = {{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}};
    const size_t gridSz = 100;
    auto grid = std::make_unique<double[]>(gridSz * gridSz);
    double xps[gridSz];
    double yps[gridSz];
    double x1 = -4, y1 = -4, x2 = 6, y2 = 6;
    double dx = (x2 - x1) / gridSz, dy = (y2 - y1) / gridSz;
    double maxV = 0.0;
    for (size_t ix = 0; ix < gridSz; ++ix) {
      auto px = x1 + ix * dx;
      xps[ix] = px;
      for (size_t iy = 0; iy < gridSz; ++iy) {
        auto py = y1 + iy * dy;
        if (ix == 0) {
          yps[iy] = py;
        }
        RDGeom::Point2D loc(px, py);
        double val = 0.0;
        for (const auto &pt : pts) {
          auto dv = loc - pt;
          auto r = dv.length();
          if (r > 0) {
            val += 1 / r;
          }
        }
        // to make the contours more visible, we cap the max val we set at 1000
        maxV = std::max(std::min(val, 1000.), maxV);
        grid[ix * gridSz + iy] = val;
      }
    }
    std::vector<conrec::ConrecSegment> segs;
    const size_t nContours = 10;
    double isoLevels[nContours];
    for (size_t i = 0; i < nContours; ++i) {
      isoLevels[i] = (i + 1) * (maxV / (nContours + 1));
    }
    conrec::Contour(grid.get(), 0, gridSz - 1, 0, gridSz - 1, xps, yps,
                    nContours, isoLevels, segs);

    std::ofstream outs("./blah.svg");
    outs << R"SVG(<?xml version='1.0' encoding='iso-8859-1'?>
<svg version='1.1' baseProfile='full'
              xmlns='http://www.w3.org/2000/svg'
                      xmlns:rdkit='http://www.rdkit.org/xml'
                      xmlns:xlink='http://www.w3.org/1999/xlink'
                  xml:space='preserve'
width='300px' height='300px' >
<rect style='opacity:1.0;fill:#FFFFFF;stroke:none' width='300' height='300' x='0' y='0'> </rect>
<!-- END OF HEADER -->
)SVG";
    for (const auto &seg : segs) {
      outs << "<path d='M " << 40 * seg.p1.x + 150 << "," << 40 * seg.p1.y + 150
           << " " << 40 * seg.p2.x + 150 << "," << 40 * seg.p2.y + 150
           << "' "
              "style='fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:"
              "0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:"
              "1' "
              "/>"
           << std::endl;
      ;
    }
    outs << "</svg>" << std::endl;
  }
}

TEST_CASE("connectLineSegments", "[conrec]") {
  SECTION("basics") {
    std::vector<RDGeom::Point2D> pts = {{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}};
    const size_t gridSz = 100;
    auto grid = std::make_unique<double[]>(gridSz * gridSz);
    double xps[gridSz];
    double yps[gridSz];
    double x1 = -4, y1 = -4, x2 = 6, y2 = 6;
    double dx = (x2 - x1) / gridSz, dy = (y2 - y1) / gridSz;
    double maxV = 0.0;
    for (size_t ix = 0; ix < gridSz; ++ix) {
      auto px = x1 + ix * dx;
      xps[ix] = px;
      for (size_t iy = 0; iy < gridSz; ++iy) {
        auto py = y1 + iy * dy;
        if (ix == 0) {
          yps[iy] = py;
        }
        RDGeom::Point2D loc(px, py);
        double val = 0.0;
        for (const auto &pt : pts) {
          auto dv = loc - pt;
          auto r = dv.length();
          if (r > 0) {
            val += 1 / r;
          }
        }
        maxV = std::max(std::min(val, 1000.), maxV);
        grid[ix * gridSz + iy] = val;
      }
    }
    std::vector<conrec::ConrecSegment> segs;
    const size_t nContours = 10;
    double isoLevels[nContours];
    for (size_t i = 0; i < nContours; ++i) {
      isoLevels[i] = (i + 1) * (maxV / (nContours + 1));
    }
    conrec::Contour(grid.get(), 0, gridSz - 1, 0, gridSz - 1, xps, yps,
                    nContours, isoLevels, segs);

    auto contours = conrec::connectLineSegments(segs);
    CHECK(contours.size() == 74);

    std::ofstream outs("./blah.contour.svg");
    outs << R"SVG(<?xml version='1.0' encoding='iso-8859-1'?>
<svg version='1.1' baseProfile='full'
              xmlns='http://www.w3.org/2000/svg'
                      xmlns:rdkit='http://www.rdkit.org/xml'
                      xmlns:xlink='http://www.w3.org/1999/xlink'
                  xml:space='preserve'
width='300px' height='300px' >
<rect style='opacity:1.0;fill:#FFFFFF;stroke:none' width='300' height='300' x='0' y='0'> </rect>
<!-- END OF HEADER -->
)SVG";
    for (const auto &pr : contours) {
      auto [contour, val] = pr;
      REQUIRE(contour.size());
      outs << "<path d='M " << 40 * contour[0].x + 150 << ","
           << 40 * contour[0].y + 150;
      for (auto i = 1u; i < contour.size(); ++i) {
        outs << " L " << 40 * contour[i].x + 150 << ","
             << 40 * contour[i].y + 150;
      }
      outs << "' "
              "style='fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:"
              "0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:"
              "1' "
              "/>"
           << std::endl;
    }
    outs << "</svg>" << std::endl;
  }
}

TEST_CASE("super chunky", "[conrec]") {
  // an example where you can really see any holes in the contours
  SECTION("basics") {
    std::vector<RDGeom::Point2D> pts = {{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}};
    const size_t gridSz = 5;
    auto grid = std::make_unique<double[]>(gridSz * gridSz);
    double xps[gridSz];
    double yps[gridSz];
    double x1 = -4, y1 = -4, x2 = 6, y2 = 6;
    double dx = (x2 - x1) / gridSz, dy = (y2 - y1) / gridSz;
    double maxV = 0.0;
    for (size_t ix = 0; ix < gridSz; ++ix) {
      auto px = x1 + ix * dx;
      xps[ix] = px;
      for (size_t iy = 0; iy < gridSz; ++iy) {
        auto py = y1 + iy * dy;
        if (ix == 0) {
          yps[iy] = py;
        }
        RDGeom::Point2D loc(px, py);
        double val = 0.0;
        for (const auto &pt : pts) {
          auto dv = loc - pt;
          auto r = dv.length();
          if (r > 0) {
            val += 1 / r;
          }
        }
        maxV = std::max(val, maxV);
        grid[ix * gridSz + iy] = val;
      }
    }
    std::vector<conrec::ConrecSegment> segs;
    const size_t nContours = 1;
    double isoLevels[nContours];
    for (size_t i = 0; i < nContours; ++i) {
      isoLevels[i] = (i + 1) * (maxV / (nContours + 1));
    }

    conrec::Contour(grid.get(), 0, gridSz - 1, 0, gridSz - 1, xps, yps,
                    nContours, isoLevels, segs);

    std::ofstream outs("./chunky.svg");
    outs << R"SVG(<?xml version='1.0' encoding='iso-8859-1'?>
<svg version='1.1' baseProfile='full'
              xmlns='http://www.w3.org/2000/svg'
                      xmlns:rdkit='http://www.rdkit.org/xml'
                      xmlns:xlink='http://www.w3.org/1999/xlink'
                  xml:space='preserve'
width='300px' height='300px' >
<rect style='opacity:1.0;fill:#FFFFFF;stroke:none' width='300' height='300' x='0' y='0'> </rect>
<!-- END OF HEADER -->
)SVG";
    for (const auto &seg : segs) {
      outs << "<path d='M " << 40 * seg.p1.x + 150 << "," << 40 * seg.p1.y + 150
           << " " << 40 * seg.p2.x + 150 << "," << 40 * seg.p2.y + 150
           << "' "
              "style='fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:"
              "0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:"
              "1' "
              "/>"
           << std::endl;
      ;
    }
    outs << "</svg>" << std::endl;

    auto contours = conrec::connectLineSegments(segs);
    std::ofstream outs2("./chunky.contour.svg");
    outs2 << R"SVG(<?xml version='1.0' encoding='iso-8859-1'?>
<svg version='1.1' baseProfile='full'
              xmlns='http://www.w3.org/2000/svg'
                      xmlns:rdkit='http://www.rdkit.org/xml'
                      xmlns:xlink='http://www.w3.org/1999/xlink'
                  xml:space='preserve'
width='300px' height='300px' >
<rect style='opacity:1.0;fill:#FFFFFF;stroke:none' width='300' height='300' x='0' y='0'> </rect>
<!-- END OF HEADER -->
)SVG";
    for (const auto &pr : contours) {
      auto [contour, val] = pr;
      REQUIRE(contour.size());
      outs2 << "<path d='M " << 40 * contour[0].x + 150 << ","
            << 40 * contour[0].y + 150;
      for (auto i = 1u; i < contour.size(); ++i) {
        outs2 << " L " << 40 * contour[i].x + 150 << ","
              << 40 * contour[i].y + 150;
      }
      outs2 << "' "
               "style='fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:"
               "0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:"
               "1' "
               "/>"
            << std::endl;
    }
    outs2 << "</svg>" << std::endl;
  }
}
