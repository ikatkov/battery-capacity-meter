import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

# Define the fitting functions


def func4(x, a, b, c, d, e):
    return a * x**4 + b * x**3 + c * x**2 + d*x + e


def func3(x, a, b, c, d):
    return a * x**3 + b * x**2 + c * x + d


def func2(x, a, b, c):
    return a * x**2 + b * x + c


def func1(x, a, b):
    return a * x + b


# Load the data from the CSV file
# data = np.loadtxt("tp4056-data-500mA.csv", delimiter=",")
data = np.loadtxt("tp4056-data-1A.csv", delimiter=",")

# Extract the x and y data from the data array
x = data[:, 0]
y = data[:, 1]

# Fit the functions to the data using curve_fit
popt4, _ = curve_fit(func4, x, y)
popt3, _ = curve_fit(func3, x, y)
popt2, _ = curve_fit(func2, x, y)
popt1, _ = curve_fit(func1, x, y)

# Plot the data and the fitted functions
x_plot = np.linspace(0, 1, 100)
plt.plot(x, y, 'o', label='data')
plt.plot(x_plot, func4(x_plot, *popt4), label='4rd-degree polynomial fit')
plt.plot(x_plot, func3(x_plot, *popt3), label='3rd-degree polynomial fit')
plt.plot(x_plot, func2(x_plot, *popt2), label='2nd-degree polynomial fit')
plt.plot(x_plot, func1(x_plot, *popt1), label='linear fit')
plt.legend()

plt.xlabel('I, mA')
plt.ylabel('V')
plt.title('Fitted Function')


print("Fitted parameters for 4th-degree polynomial: ",
      f"{popt4[0]:.8f}*pow(voltage, 4) + {popt4[1]:.8f}*pow(voltage, 3) + {popt4[2]:.8f}*pow(voltage, 2) + {popt4[3]:.8f}*voltage + {popt4[4]:.8f}")
print("Fitted parameters for 3rd-degree polynomial: ",
      f"{popt3[0]:.8f}*pow(voltage, 3) + {popt3[1]:.8f}*pow(voltage, 2) + {popt3[2]:.8f}*voltage + {popt3[3]:.8f}")
print("Fitted parameters for 2nd-degree polynomial: ",
      f"{popt2[0]:.8f}*pow(voltage, 2) + {popt2[1]:.8f}*voltage + {popt2[2]:.8f}")
print("Fitted parameters for linear function: ",
      f"{popt1[0]:.8f}*voltage + {popt1[1]:.8f}")


# plt.show()

x_values = np.arange(0, 1.1, 0.1)

# Print the 4th-degree polynomial for x and y values
for x in x_values:
    y_value = func4(x, *popt4)
    print(f"For x = {x:.1f}, y = {y_value:.8f}")
