import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
import time

def f(x, y):
    return x ** 3 * y ** 5 * (4 - x - 7 * y)

def grad_f(x, y):
    df_dx = x ** 2 * y ** 5 * (12 - 4 * x - 21 * y)
    df_dy = x ** 3 * y ** 4 * (20 - 5 * x - 42 * y)
    return np.array([df_dx, df_dy])

def gradient_descent(start, ak=1e-4, eps=1e-6, max_iterations=100000):
    way, grad = [np.array(start)], None
    for _ in range(max_iterations):
        grad = grad_f(*way[-1])
        new_point = way[-1] + ak * grad
        way.append(new_point)
        if np.linalg.norm(new_point - way[-2]) < eps:
            return way, "Convergence"
    return way, "Max Iterations Reached"

starting_point = [2, 2]
start_time = time.time()
path, break_condition = gradient_descent(starting_point)
end_time = time.time()

path_x, path_y = zip(*path)
x_expanded, y_expanded = np.linspace(-2, 2, 600), np.linspace(-2, 2, 600)
x_expanded, y_expanded = np.meshgrid(x_expanded, y_expanded)
z_expanded = f(x_expanded, y_expanded)

def get_color_gradient(n, start_color, end_color):
    return [start_color + (end_color - start_color) * i / n for i in range(n)]

color_gradient = get_color_gradient(len(path), np.array([0, 0, 1]), np.array([1, 0, 0]))

fig = plt.figure(figsize=(16, 8))
ax1, ax2 = fig.add_subplot(121, projection='3d'), fig.add_subplot(122)

ax1.plot_surface(x_expanded, y_expanded, z_expanded, cmap=cm.coolwarm, alpha=0.7)
ax1.set_title('Plot')
ax2.contour(x_expanded, y_expanded, z_expanded, 100, cmap=cm.coolwarm)

for i in range(0, len(path), 100):
    ax1.scatter(path_x[i], path_y[i], f(path_x[i], path_y[i]), color=color_gradient[i], alpha=0.6, s=50)
    ax2.scatter(path_x[i], path_y[i], color=color_gradient[i], alpha=0.6, s=50)

ax2.scatter([starting_point[0]], [starting_point[1]], color='blue', s=100, label='Starting Point')
ax2.scatter([path_x[-1]], [path_y[-1]], color='green', s=100, label='Ending Point')
ax2.set_title('Contour Plot with Enhanced Path')
ax2.set_xlabel('x')
ax2.set_ylabel('y')
ax2.legend()

plt.tight_layout()
plt.show()


final_point = path[-1]
final_value = f(*final_point)
exact_point = (4/3, 20/63)
exact_value = f(*exact_point)
execution_time = end_time - start_time

print(f"Results:\n"
      f"- Break Condition: {break_condition}\n"
      f"- Final Point: {final_point}\n"
      f"- Function Value at Final Point: {final_value:.6f}\n"
      f"- Exact Point: {exact_point}\n"
      f"- Function Value at Exact Point: {exact_value:.6f}\n"
      f"- Difference between Final and Exact Values: {final_value - exact_value:.6f}\n"
      f"- Execution Time: {execution_time} seconds\n"
      f"- Number of Iterations: {len(path)}")
