// Copyright (c) 2014 Max Planck Society

/*!@file
 * @author  Stephan Wenninger <swenninger@tuebingen.mpg.de>
 *
 * @date    2014-10-30
 *
 * @brief
 * The file holds a CircularBuffer class.
 *
 */

#ifndef PHD_CIRCULAR_BUFFER
#define PHD_CIRCULAR_BUFFER

#include <Eigen/Dense>

/*!
 * The CircularDoubleBuffer class provides a double vector of limited size.
 * If the vector is full, new data will be appended to the front of the vector, 
 * overriding any previous data.
 *
 * Usage:
 * @code
 *  int max_size = 10;
 *  CircularDoubleBuffer buffer(max_size);
 *  for (int i = 0; i < 20; ++i {
 *    buffer.append(i);
 *  }
 * @endcode
 *
 * When passing the Vector to functions that expect an
 * @code Eigen::VectorXd& @endcode use:
 * @code buffer.getEigenVector() @endcode
 *
 * @note Using @code buffer.get(int) @endcode expects that append has been called
 * often enough!
 *
 */
class CircularDoubleBuffer {
 private:
  int max_size_;
  int current_position_;
  Eigen::VectorXd buffer_;
  Eigen::VectorXd tmp_;
  bool max_size_exceeded_;

 public:
  /*!
   * Constructor of the CircularBuffer class.
   *
   * @param max_size The maximum size of the Buffer.
   */
  explicit CircularDoubleBuffer(int max_size);

  ~CircularDoubleBuffer() {}

  /*!
   * Returns the element at given index.
   *
   * @param index The index of the element in the vector
   * @return The element at the given index
   * @pre The index must fulfill: 0 <= index < max_size_
   */
  double get(int index);

  /*!
   * Appends a datapoint to the buffer. Potentially overriding previous data,
   * if @code append(data) @endcode was called more than 
   * @code max_size_ @endcode times.
   * 
   * @param data The datapoint that will be appended
   */
  void append(double data);

  /*!
   * Returns the index where the last element was appended to.
   */
  int lastElementIndex();


  /*!
   * Clears the buffer, by setting all values to 0.0 and resetting the current
   * position to 0
   */
  void clear();

  /*!
   * Returns (a pointer to) the underlying Eigen::VectorXd object. If max_size_
   * has not been exceeded so far, the vector is trimmed to represent only the
   * vector of already appended data.
   * 
   * Say: 
   * @code
   * CircularBuffer buffer(20);
   * for (int i = 0; i < 10; ++i) {
   *  buffer.append(i);
   * }
   * @endcode
   * Then this function 
   */
  Eigen::VectorXd* getEigenVector();
};


#endif  // PHD_CIRCULAR_BUFFER
