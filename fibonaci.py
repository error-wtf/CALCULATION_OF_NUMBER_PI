import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# Function to generate Fibonacci logarithmic spiral points
def fibonacci_log_spiral(n_points, scale):
    # Generate theta values that follow the Fibonacci sequence in growth
    theta = np.linspace(0, 4 * np.pi, n_points)
    radius = scale * np.exp(0.30635 * theta)  # 0.30635 approximates the growth rate of the Fibonacci sequence
    x = radius * np.cos(theta)
    y = radius * np.sin(theta)
    return x, y

# Display initial Fibonacci logarithmic spiral
n_points = 500  # Number of points in the spiral
scale = 1  # Scale factor for the spiral

x, y = fibonacci_log_spiral(n_points, scale)
fig, ax = plt.subplots()
spiral_line, = ax.plot(x, y, color="blue")
ax.set_aspect("equal", "box")
plt.title("Fibonacci Logarithmic Spiral")
plt.grid(True)

# Update function for interactive sliders
def update(val):
    n_points = int(points_slider.val)
    scale = scale_slider.val
    
    x, y = fibonacci_log_spiral(n_points, scale)
    spiral_line.set_data(x, y)
    ax.relim()
    ax.autoscale_view()
    fig.canvas.draw_idle()

# Slider for the number of points
points_ax = plt.axes([0.2, 0.01, 0.65, 0.03])
points_slider = Slider(points_ax, 'Points', 100, 1000, valinit=n_points, valstep=10)
points_slider.on_changed(update)

# Slider for the scale factor
scale_ax = plt.axes([0.2, 0.05, 0.65, 0.03])
scale_slider = Slider(scale_ax, 'Scale', 0.1, 5, valinit=scale)
scale_slider.on_changed(update)

plt.show()


