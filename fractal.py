import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, RadioButtons

# Function to display the Mandelbrot set
def mandelbrot(c, max_iter):
    z = c
    for n in range(max_iter):
        if abs(z) > 2:
            return n
        z = z * z + c
    return max_iter

# Fractal generation functions
def generate_fractal(fractal_type, x_min, x_max, y_min, y_max, width, height, max_iter, progress_callback, c=0+0j):
    x, y = np.linspace(x_min, x_max, width), np.linspace(y_min, y_max, height)
    X, Y = np.meshgrid(x, y)
    Z = X + 1j * Y
    fractal = np.zeros(Z.shape, dtype=int)
    
    for i in range(width):
        for j in range(height):
            if fractal_type == "Mandelbrot":
                fractal[j, i] = mandelbrot(Z[j, i], max_iter)
            elif fractal_type == "Julia":
                fractal[j, i] = mandelbrot(Z[j, i] + c, max_iter)
            elif fractal_type == "Burning Ship":
                Z[j, i] = abs(Z[j, i].real) + 1j * abs(Z[j, i].imag)
                fractal[j, i] = mandelbrot(Z[j, i], max_iter)
            elif fractal_type == "Tricorn":
                Z[j, i] = np.conj(Z[j, i])**2 + Z[j, i]
                fractal[j, i] = mandelbrot(Z[j, i], max_iter)
            elif fractal_type == "Newton":
                z = Z[j, i]
                for n in range(max_iter):
                    dz = (z**3 - 1) / (3 * z**2)
                    z -= dz
                    if abs(dz) < 1e-6:
                        fractal[j, i] = n
                        break
                else:
                    fractal[j, i] = max_iter

        # Update progress
        progress_callback((i + 1) / width * 100)
    
    return fractal

# Display initial Mandelbrot fractal
width, height = 800, 800
x_min, x_max, y_min, y_max = -2.0, 1.0, -1.5, 1.5
max_iter = 50
fractal_type = "Mandelbrot"

# Progress Window
def show_progress(progress):
    if plt.fignum_exists(progress_fig.number):
        progress_ax.clear()
        progress_ax.set_xlim(0, 100)
        progress_ax.barh(0, progress, color="green")  # Set the color to green
        progress_ax.set_title("Calculating...")
        progress_ax.set_xlabel("Progress (%)")
        progress_fig.canvas.draw()
        plt.pause(0.01)

# Main fractal window
fig, ax = plt.subplots()
im = ax.imshow(generate_fractal(fractal_type, x_min, x_max, y_min, y_max, width, height, max_iter, lambda x: None),
               cmap='twilight', extent=[x_min, x_max, y_min, y_max])
plt.colorbar(im, ax=ax)

# Update function for interactive sliders
def update(val):
    global fractal_type, progress_fig, progress_ax
    max_iter = iter_slider.val
    fractal_type = radio_fractal.value_selected

    # Show progress window
    progress_fig, progress_ax = plt.subplots(figsize=(6, 2))
    progress_fig.canvas.manager.set_window_title("Progress")
    plt.ion()
    plt.show()

    # Generate and display new fractal with progress
    fractal_data = generate_fractal(fractal_type, x_min, x_max, y_min, y_max, width, height, int(max_iter), show_progress)
    plt.close(progress_fig)  # Close progress window once done
    im.set_data(fractal_data)
    fig.canvas.draw_idle()

# Sliders for iterations
iter_ax = plt.axes([0.2, 0.01, 0.65, 0.03])
iter_slider = Slider(iter_ax, 'Iterations', 10, 200, valinit=max_iter)
iter_slider.on_changed(update)

# Radio buttons for fractal type selection
radio_ax = plt.axes([0.02, 0.5, 0.15, 0.3])
radio_fractal = RadioButtons(radio_ax, ('Mandelbrot', 'Julia', 'Burning Ship', 'Tricorn', 'Newton'))
radio_fractal.on_clicked(update)

# Event loop to keep the window responsive
while plt.fignum_exists(fig.number):
    plt.pause(0.1)
