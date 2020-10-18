#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <tuple>
#include <vector>

//////////////////////// Math helpers /////////////////////////////

static constexpr double minDivision = 1.e-64;
static constexpr double minComparison = 1.e-12;

static inline double getRadians(double deg) { return deg * M_PI / 180.0; }

//////////////////////// Point implementation ////////////////////

struct Line;

struct Point {
  double x;
  double y;

  explicit Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}

  bool operator==(const Point &rhs) const {
    return fabs(x - rhs.x) < minComparison && fabs(y - rhs.y) < minComparison;
  }

  bool operator!=(const Point &rhs) const { return !(this->operator==(rhs)); }

  // rotate by an angle (in degrees, counterclockwise) relative to a point
  // https://math.stackexchange.com/questions/814950/how-can-i-rotate-a-coordinate-around-a-circle
  void rotate(Point O, double radian) {
    double xDiff = (x - O.x), yDiff = (y - O.y);
    x = std::cos(radian) * xDiff - std::sin(radian) * yDiff + O.x;
    y = std::sin(radian) * xDiff + std::cos(radian) * yDiff + O.y;
  }

  // symmetry with respect to a point
  void reflex(Point O) {
    x = 2 * O.x - x;
    y = 2 * O.y - y;
  }

  // symmetry with respect to a line
  // https://en.wikipedia.org/wiki/Point_reflection
  void reflex(Line axis);

  // homothety with ratio coefficient and center
  void scale(Point O, double coeff) {
    x = O.x + coeff * (x - O.x);
    y = O.y + coeff * (y - O.y);
  }
};

double dist(const Point &A, const Point &B) {
  return std::sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}

//////////////////////// Line implementation /////////////////////////////

struct Line {
  // line equation:
  // a * y + b * x + c = 0
  // where a = 1 or a = 0
  double a{};
  double b{};
  double c{};

  // input: 2 points A, B on line
  explicit Line(Point A, Point B) {
    // a * (y1 - y2) + b * (x1 - x2) = 0
    if (fabs(A.y - B.y) < minDivision) {
      // horizontal line
      a = 1.;
      b = 0.;
      c = -A.y;
    } else if (fabs(A.x - B.x) < minDivision) {
      // vertical line
      a = 0;
      b = 1;
      c = -A.x;
    } else {
      // general case
      a = 1.;
      b = -(A.y - B.y) / (A.x - B.x);
      c = -(a * A.y + b * A.x);
    }
  }

  // input: y = alpha * x + shift, alpha and shift are given
  explicit Line(double alpha, double shift) {
    a = 1.;
    b = -alpha;
    c = -shift;
  }

  // input: y = alpha * x + shift, alpha and point on line are given
  explicit Line(Point A, double alpha) {
    a = 1.;
    b = -alpha;
    c = alpha * A.x - A.y;
  }

  bool operator==(const Line &rhs) const {
    return fabs(a - rhs.a) < minComparison && fabs(b - rhs.b) < minComparison &&
           fabs(c - rhs.c) < minComparison;
  }

  bool operator!=(const Line &rhs) const { return !this->operator==(rhs); }
};

void Point::reflex(Line axis) {
  double sqrDiff = axis.a * axis.a - axis.b * axis.b;
  double sqrSum = axis.a * axis.a + axis.b * axis.b;
  (*this) = Point((x * sqrDiff - 2 * axis.b * (axis.a * y + axis.c)) / sqrSum,
                  (-y * sqrDiff - 2 * axis.a * (axis.b * x + axis.c)) / sqrSum);
}

std::optional<Point> intersection(Line l1, Line l2) {
  if (l1.a == l2.a && l1.b == l2.b && l1.c != l2.c) {
    // parallel lines
    return {};
  } else {
    // a1 y + b1 x + c1 = 0;
    // a2 y + b2 x + c2 = 0;
    if (fabs(l1.a) < minDivision) {
      // x + c1 = 0;
      // a2 y + b2 x + c2 = 0, a2 > 0 (not parallel)
      double x = -l1.c;
      double y = (-l2.c - l2.b * x) / l2.a;
      return {Point(x, y)};
    } else if (fabs(l1.b) < minDivision) {
      // y + c1 = 0;
      // a2 y + b2 x + c2 = 0, b2 > 0 (not parallel)
      double y = -l1.c;
      double x = (-l2.c - l2.a * y) / l2.b;
      return {Point(x, y)};
    } else {
      // a1 y + b1 x + c1 = 0; => x = (-c1 - a1 y) / b1
      // a2 y + b2 x + c2 = 0; => (a2 - a1 * b2 / b1) y = c1 * b2 / b1 - c2
      double y = (l1.c * l2.b / l1.b - l2.c) / (l2.a - l1.a * l2.b / l1.b);
      double x = (-l1.c - l1.a * y) / l1.b;
      return {Point(x, y)};
    }
  }
}

//////////////////////////// Shape definition /////////////////////////////

class Shape {
 protected:
  // auxillary params
  mutable std::optional<double> mArea;
  mutable std::optional<double> mPerimeter;

  virtual void invalidateAxillaryParams() {
    mArea = {};
    mPerimeter = {};
  }

 public:
  virtual bool operator==(const Shape &rhs) const = 0;
  virtual bool operator!=(const Shape &rhs) const = 0;

  virtual double perimeter() const = 0;
  virtual double area() const = 0;

  // rotate by an angle (in degrees, counterclockwise) relative to a point
  virtual void rotate(Point center, double angle) = 0;

  // symmetry with respect to a point
  virtual void reflex(Point center) = 0;

  // symmetry with respect to a line
  virtual void reflex(Line axis) = 0;

  // homothety with ratio coefficient and center
  virtual void scale(Point center, double coefficient) = 0;
};

////////////////////// Polygon implementation //////////////////////////

class Polygon : public Shape {
 protected:
  // points have counterclockwise orientation (it's guaranteed by constructor)
  // first points have the lowest coordinates to fix order
  std::vector<Point> mPoints;

  void fixOrientation() {
    static constexpr size_t minVerticesForOrientation = 3;
    if (mPoints.size() >= minVerticesForOrientation) {
      auto &P = *std::begin(mPoints);
      auto &Q = *std::next(std::begin(mPoints));
      auto &R = *std::next(std::next(std::begin(mPoints)));
      // z-th coordinate of normal to PQR, need to check sgn
      if (((Q.y - P.y) * (R.x - Q.x) - (Q.x - P.x) * (R.y - Q.y)) >= 0) {
        // negative orientation: fix it to counterclockwise
        std::reverse(std::begin(mPoints), std::end(mPoints));
      }
      // change order to fixed if needed
      auto it = std::min_element(std::begin(mPoints), std::end(mPoints),
                                 [](const Point &lhs, const Point &rhs) {
                                   return std::tie(lhs.x, lhs.y) <
                                          std::tie(rhs.x, rhs.y);
                                 });
      if (it != std::begin(mPoints)) {
        // rotation is needed to maintain fixed order
        std::rotate(std::begin(mPoints), it, std::end(mPoints));
      }
    }
  }

 public:
  explicit Polygon(std::vector<Point> points = {})
      : mPoints(std::move(points)) {
    fixOrientation();
  }

  virtual size_t verticesCount() const final { return mPoints.size(); }

  virtual std::vector<Point> getVertices() const final { return mPoints; }

  // General shape methods overriding

  bool operator==(const Shape &rhs) const final {
    return mPoints == dynamic_cast<const Polygon &>(rhs).mPoints;
  }

  bool operator!=(const Shape &rhs) const final {
    return mPoints != dynamic_cast<const Polygon &>(rhs).mPoints;
  }

  // P = sum_{i = 0 ... N-1} dist(A_i, A_{i+1})
  double perimeter() const final {
    static constexpr size_t perimeterMinVertices = 1;
    if (!mPerimeter.has_value()) {
      double p = 0;
      if (verticesCount() > perimeterMinVertices) {
        auto first = std::begin(mPoints);
        auto prev = first;
        for (auto it = std::next(prev); it != std::end(mPoints); it++) {
          p += dist(*prev, *it);
          prev = it;
        }
        p += dist(*prev, *first);
      }
      mPerimeter = {p};
    }
    return mPerimeter.value();
  }

  // A = 0.5 * sum_{i = 0 ... N-1} (A_i.x * A_{i+1}.y - A_{i+1}.x * A_i.y)
  // Note: method will produce the wrong answer for self-intersecting
  // polygons
  double area() const final {
    static constexpr size_t areaMinVertices = 2;
    if (!mArea.has_value()) {
      double a = 0;
      if (verticesCount() > areaMinVertices) {
        auto first = std::begin(mPoints);
        auto prev = first;
        for (auto it = std::next(prev); it != std::end(mPoints); it++) {
          a += (prev->x * it->y - it->x * prev->y);
          prev = it;
        }
        a += (prev->x * first->y - first->x * prev->y);
      }
      mArea = {0.5 * a};
    }
    return mArea.value();
  }

  // Rotate by an angle (in degrees, counterclockwise) relative to a point
  // Every point P_i moves on circle (O, R), where R = dist(O, P_i)
  // source:
  // https://math.stackexchange.com/questions/814950/how-can-i-rotate-a-coordinate-around-a-circle
  void rotate(Point O, double angle) final {
    const double radian = getRadians(angle);
    for (auto &P : mPoints) {
      P.rotate(O, radian);
    }
  }

  // Symmetry with respect to a point,
  // Should reflect each point: P_reflected = O + 2 * vec{PO}
  // reflex(O) == scale(O, -2), but direct formula has less arithmetic
  // operations
  void reflex(Point O) final {
    for (auto &P : mPoints) {
      P.reflex(O);
    }
  }

  // Symmetry with respect to a point,
  // Should reflect each point: P_reflected = O + 2 * vec{PO}
  // source: https://math.stackexchange.com/a/1743581/675968
  void reflex(Line axis) final {
    for (auto &P : mPoints) {
      P.reflex(axis);
    }
  }

  // Homothety with ratio coefficient and center,
  // Should send each point P -> O + coeff(P - O)
  // source https://en.wikipedia.org/wiki/Homothetic_transformation
  void scale(Point O, double coeff) final {
    for (auto &P : mPoints) {
      P.scale(O, coeff);
    }
    invalidateAxillaryParams();
  }
};

//////////////////////// Ellipse implementation /////////////////////////////

// https://en.wikipedia.org/wiki/Ellipse
class Ellipse : public Shape {
 protected:
  // these parameters describe ellipse
  Point mF1{};
  Point mF2{};
  double mA{};

  // special auxillary parameters
  mutable std::optional<double> mB;  //< semi-minor axis
  mutable std::optional<double> mC;  //< foki dist
  mutable std::optional<double> mE;  //< eccentricity

  void invalidateAxillaryParams() override {
    Shape::invalidateAxillaryParams();
    mB = {};
    mC = {};
    mE = {};
  }

 public:
  // Ellipse: all points M such that |F1 M| + |F2 M| = 2 * a = const,
  // where F1, F2 are called foci points
  explicit Ellipse(Point F1, Point F2, double doubledA)
      : mF1(F1), mF2(F2), mA(0.5 * doubledA) {}

  virtual std::pair<Point, Point> focuses() const final { return {mF1, mF2}; }

  virtual double semiMajorAxis() const final { return mA; }

  virtual double semiMinorAxis() const final {
    if (!mB.has_value()) {
      mB = {mA * std::sqrt(1 - eccentricity() * eccentricity())};
    }
    return mB.value();
  }

  virtual double fociDistance() const final {
    if (!mC.has_value()) {
      mC = {dist(mF1, mF2)};
    }
    return mC.value();
  }

  virtual double eccentricity() const final {
    if (!mE.has_value()) {
      mE = {0.5 * fociDistance() / mA};
    }
    return mE.value();
  }

  virtual Point center() const final {
    return Point(0.5 * (mF1.x + mF2.x), 0.5 * (mF1.y + mF2.y));
  }

  bool operator==(const Shape &rhs) const final {
    auto &that = dynamic_cast<const Ellipse &>(rhs);
    return mF1 == that.mF1 && mF2 == that.mF2 &&
           fabs(mA - that.mA) < minComparison;
  }

  bool operator!=(const Shape &rhs) const final {
    return !(this->operator==(rhs));
  }

  // https://en.wikipedia.org/wiki/Elliptic_integral#Complete_elliptic_integral_of_the_second_kind
  // https://en.cppreference.com/w/cpp/numeric/special_functions/comp_ellint_2
  double perimeter() const override {
    if (!mPerimeter.has_value()) {
      mPerimeter = {4 * mA * std::comp_ellint_2(eccentricity())};
    }
    return mPerimeter.value();
  }

  double area() const override {
    if (!mArea.has_value()) {
      mArea = {M_PI * mA * semiMinorAxis()};
    }
    return mArea.value();
  }

  // rotate by an angle (in degrees, counterclockwise) relative to a point
  void rotate(Point O, double angle) final {
    const double radian = getRadians(angle);
    mF1.rotate(O, radian);
    mF2.rotate(O, radian);
    // mA and auxillary params don't change
  }

  // symmetry with respect to a point
  void reflex(Point O) final {
    mF1.reflex(O);
    mF2.reflex(O);
    // mA and auxillary params don't change
  }

  // symmetry with respect to a line
  void reflex(Line axis) final {
    mF1.reflex(axis);
    mF2.reflex(axis);
    // mA and auxillary params don't change
  }

  // homothety with ratio coefficient and center
  void scale(Point O, double coeff) final {
    mF1.scale(O, coeff);
    mF2.scale(O, coeff);
    mA *= fabs(coeff);
    invalidateAxillaryParams();
  }
};

/////////////////////// Circle implementation ////////////////////////

class Circle : public Ellipse {
 public:
  explicit Circle(Point O, double R) : Ellipse(O, O, 2 * R) {}

  double radius() const { return semiMajorAxis(); }

  double perimeter() const override {
    if (!mPerimeter.has_value()) {
      mPerimeter = {2 * M_PI * radius()};
    }
    return mPerimeter.value();
  }

  double area() const override {
    if (!mArea.has_value()) {
      mArea = {M_PI * radius() * radius()};
    }
    return mArea.value();
  }
};

/////////////////////// Rectangle implementation ////////////////////////

class Rectangle : public Polygon {
 protected:
  static constexpr size_t indexA = 0;
  static constexpr size_t indexB = indexA + 1;
  static constexpr size_t indexC = indexB + 1;
  static constexpr size_t indexD = indexC + 1;

 public:
  explicit Rectangle(std::vector<Point> points) : Polygon(move(points)) {
    // Orientation is fixed in base class constructor
  }

  // A, C - opposite vertices, ratio = AB / BC
  // construct in local coordinate system with parallel axis,
  // then rotate to target coordinate system around point A
  explicit Rectangle(Point A, Point C, double ratio) {
    const double lenDiag = dist(A, C);
    const double targetAngleCAX = std::acos(fabs(A.x - C.x) / lenDiag);
    const double radianAngleCAD = std::atan(ratio);  //< radians

    // rotate to local coordinate system with axis parallel to OX, OY
    const double rotationAngle = radianAngleCAD - targetAngleCAX;
    C.rotate(A, rotationAngle);

    // now it is easy to construct rectangle
    Point B = Point(A.x, C.y);
    Point D = Point(C.x, A.y);
    mPoints = {A, B, C, D};
    fixOrientation();

    // return to target coordinate system
    rotate(A, -rotationAngle);
  }

  [[nodiscard]] Point center() const {
    const auto &A = mPoints[indexA], &C = mPoints[indexC];
    return Point(0.5 * (A.x + C.x), 0.5 * (A.y + C.y));
  }

  std::pair<Line, Line> diagonals() const {
    return {Line(mPoints[indexA], mPoints[indexC]),
            Line(mPoints[indexB], mPoints[indexD])};
  }
};

/////////////////////// Square implementation ////////////////////////

class Square : public Rectangle {
 public:
  explicit Square(std::vector<Point> points) : Rectangle(move(points)) {
    // Orientation is fixed in base class constructor
  }

  // A, C - opposite vertices
  explicit Square(Point A, Point C) : Rectangle(A, C, 1.) {
    // Orientation is fixed in base class constructor
  }

  Circle circumscribedCircle() const {
    return Circle(center(), 0.5 * dist(mPoints[indexA], mPoints[indexC]));
  }

  Circle inscribedCircle() const {
    return Circle(center(), 0.5 * dist(mPoints[indexA], mPoints[indexB]));
  }
};

/////////////////////// Triangle implementation ////////////////////////

class Triangle : public Polygon {
 protected:
  static constexpr size_t indexA = 0;
  static constexpr size_t indexB = indexA + 1;
  static constexpr size_t indexC = indexB + 1;

 public:
  explicit Triangle(Point A, Point B, Point C) : Polygon({A, B, C}) {
    // Orientation is fixed in base class constructor
  }

  explicit Triangle(std::vector<Point> points) : Polygon(move(points)) {
    // Orientation is fixed in base class constructor
  }

  Circle circumscribedCircle() const {
    auto &A = mPoints[indexA];
    auto &B = mPoints[indexB];
    auto &C = mPoints[indexC];
    double ssA = (A.x * A.x + A.y * A.y);
    double ssB = (B.x * B.x + B.y * B.y);
    double ssC = (C.x * C.x + C.y * C.y);
    double D = 2 * (A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
    double x = (ssA * (B.y - C.y) + ssB * (C.y - A.y) + ssC * (A.y - B.y)) / D;
    double y = (ssA * (C.x - B.x) + ssB * (A.x - C.x) + ssC * (B.x - A.x)) / D;
    return Circle(Point(x, y), dist(A, Point(x, y)));
  }

  // https://en.wikipedia.org/wiki/Incircle_and_excircles_of_a_triangle
  // https://math.stackexchange.com/questions/617403/how-many-ways-to-find-the-center-of-an-inscribed-circle
  Circle inscribedCircle() const {
    auto &A = mPoints[indexA];
    auto &B = mPoints[indexB];
    auto &C = mPoints[indexC];
    double a = dist(B, C);
    double b = dist(A, C);
    double c = dist(A, B);
    double p = perimeter();
    Point O = Point((a * A.x + b * B.x + c * C.x) / p,
                    (a * A.y + b * B.y + c * C.y) / p);
    return Circle(O, area() / (0.5 * p));
  }

  // https://e-maxx.ru/algo/gravity_center
  Point centroid() const {
    auto &A = mPoints[indexA];
    auto &B = mPoints[indexB];
    auto &C = mPoints[indexC];
    return Point((A.x + B.x + C.x) / 3, (A.y + B.y + C.y) / 3);
  }

  // https://en.wikipedia.org/wiki/Altitude_(triangle)#Orthocenter
  Point orthocenter() const {
    auto &A = mPoints[indexA];
    auto &B = mPoints[indexB];
    auto &C = mPoints[indexC];

    // H -- orthocenter
    // h_a -- altitude from A to BC
    // h_b -- altitude from B to AC
    // H_BC -- projection of H on BC
    // H_AC -- projection of H on AC
    // let's find lines contain h_a and h_b

    auto reflectedA = A;
    reflectedA.reflex(Line(B, C));
    auto H_BC = Point(0.5 * (A.x + reflectedA.x), 0.5 * (A.y + reflectedA.y));
    auto h_a = Line(A, H_BC);

    auto reflectedB = B;
    reflectedB.reflex(Line(A, C));
    auto H_AC = Point(0.5 * (B.x + reflectedB.x), 0.5 * (B.y + reflectedB.y));
    auto h_b = Line(B, H_AC);

    // then H = intersection of h_a and h_b
    return intersection(h_a, h_b).value();
  }

  // https://en.wikipedia.org/wiki/Euler_line
  Line EulerLine() const {
    return Line(orthocenter(), circumscribedCircle().center());
  }

  // https://en.wikipedia.org/wiki/Nine-point_circle
  Circle ninePointsCircle() const {
    auto orth = orthocenter();
    auto circum = circumscribedCircle();
    Point O = Point(0.5 * (orth.x + circum.center().x),
                    0.5 * (orth.y + circum.center().y));
    return Circle(O, 0.5 * circum.radius());
  }
};