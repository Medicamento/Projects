import math
from time import time
import matplotlib.pyplot as plt
import numpy as np

from matplotlib.patches import Rectangle


def p(x, y):
    return x ** 2 + y ** 2


def q(x, y):
    return 3 * x * y


def chord_angle(chord_length):
    return math.asin(chord_length / 2) * 2


def get_rotate_function(phi):
    def rotate(vector):
        rotation_matrix = np.array([
            [math.cos(phi), -math.sin(phi)],
            [math.sin(phi), math.cos(phi)]
        ])
        return vector.dot(rotation_matrix).round(100)

    return rotate


def calculate_integral(dots):
    return sum([p(dots[i][0], dots[i][1]) * (dots[i][0] - dots[i - 1][0]) +
                q(dots[i][0], dots[i][1]) * (dots[i][1] - dots[i - 1][1])
                for i in range(1, len(dots))])


def create_path(step):
    dots = []
    angle = math.pi / 4
    step_angle = chord_angle(step)
    x, y = math.sqrt(2) / 2, math.sqrt(2) / 2
    rotate = get_rotate_function(step_angle)

    while angle > -3 * math.pi / 4:
        dots.append((x, y))
        angle -= step_angle
        x, y = rotate(np.array([x, y]))

    x, y = -math.sqrt(2) / 2, -math.sqrt(2) / 2
    diagonal_step = step * math.sqrt(0.5)

    while x <= math.sqrt(2) / 2:
        dots.append((x, y))
        x += diagonal_step
        y += diagonal_step

    return dots


def function_f(x, y):
    return y


def is_center_inside(x, y, delta):
    return x ** 2 + y ** 2 <= 1 and x >= y


def is_all_inside(x, y, delta):
    for shift_x in [delta / 2, -delta / 2]:
        for shift_y in [delta / 2, -delta / 2]:
            if (x + shift_x) ** 2 + (y + shift_y) ** 2 > 1 or (x + shift_x) < (y + shift_y):
                return False
    return True


def is_corner_inside(x, y, delta):
    for shift_x in [delta / 2, -delta / 2]:
        for shift_y in [delta / 2, -delta / 2]:
            if (x + shift_x) ** 2 + (y + shift_y) ** 2 <= 1 and (x + shift_x) >= (y + shift_y):
                return True
    return False


def calculate_integral_sum_conditionally(x, y, delta, condition):
    if condition(x, y, delta):
        return delta ** 2 * function_f(x, y)
    return 0


def draw_partition(title, squares, delta, condition):
    plt.figure(figsize=(10, 10))
    plt.axhline(0, color='black', linewidth=0.5)
    plt.axvline(0, color='black', linewidth=0.5)
    plt.xlabel('x'), plt.ylabel('y')
    plt.title(title)
    plt.gca().set(xlim=(-1.5, 1.5), ylim=(-1.5, 1.5))

    # Create a colormap
    num_squares = len(squares)
    colors = plt.cm.plasma(np.linspace(0, 1, num_squares))

    for i, (x, y) in enumerate(squares):
        anchor = (x - delta / 2, y - delta / 2)
        color = colors[i] if condition(x, y, delta) else 'lightgrey'
        rect = Rectangle(anchor, delta, delta, color=color, alpha=0.7)
        plt.gca().add_patch(rect)


def plot_integral_path(dots, step):
    plt.figure(figsize=(10, 10))
    partition_x, partition_y = zip(*dots)

    num_points = len(dots)
    colors = plt.cm.plasma(
        np.linspace(0, 1, num_points))

    for i in range(num_points):
        plt.plot(partition_x[i], partition_y[i], marker='o', markersize=5,
                 linestyle='', color=colors[i])

    plt.xlabel('X'), plt.ylabel('Y')
    plt.title(f'Integral Path with Step = {step}')
    plt.grid(color='gray', linestyle='--', linewidth=0.5)
    plt.gca().set_aspect('equal')


def main():
    integral_value = math.sqrt(2) / 3
    deltas = [0.1, 0.01, 0.001]

    for delta in deltas:
        start_time = time()
        dots = create_path(delta)
        integral_sum = calculate_integral(dots)
        elapsed_time = time() - start_time

        print(f"Results for delta = {delta}: Integral Sum: {integral_sum}, Error: {integral_value - integral_sum}, Time: {elapsed_time}")
        plot_integral_path(dots, delta)

    plt.show()
    death_values = [0.01, 0.001]

    for delta in deltas:
        start_time = time()
        integral_sum, min_sum, max_sum = 0, 0, 0
        squares = []

        y_start = 1.5
        x_start = -2.5
        while y_start - delta >= -1.5:
            x = x_start
            while x + delta <= 1.5:
                center_x, center_y = x + delta / 2, y_start - delta / 2
                if delta not in death_values:
                    squares.append((center_x, center_y))
                integral_sum -= calculate_integral_sum_conditionally(center_x, center_y, delta, is_center_inside)
                min_sum += calculate_integral_sum_conditionally(center_x, center_y, delta, is_all_inside)
                max_sum += calculate_integral_sum_conditionally(center_x, center_y, delta, is_corner_inside)
                x += delta
            y_start -= delta

        elapsed_time = time() - start_time
        print(
            f"Results for delta = {delta}: Integral Sum: {integral_sum}, Error: {integral_value - integral_sum}, Time: {elapsed_time}, Delta (max-min): {max_sum - min_sum}")

        if delta not in death_values:
            draw_partition(f'Normal Sum Partition (delta = {delta})', squares, delta, is_center_inside)
            draw_partition(f'Minimal Sum Partition (delta = {delta})', squares, delta, is_all_inside)
            draw_partition(f'Maximal Sum Partition (delta = {delta})', squares, delta, is_corner_inside)

    plt.show()


if __name__ == "__main__":
    main()
