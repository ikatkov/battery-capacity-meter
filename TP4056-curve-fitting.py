import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

# Define the fitting functions


def func3(x, a, b, c, d):
    return a * x**3 + b * x**2 + c * x + d


def func2(x, a, b, c):
    return a * x**2 + b * x + c


def func1(x, a, b):
    return a * x + b


# Load the data from the CSV file
data = np.loadtxt("tp4056-data.csv", delimiter=",")

# Extract the x and y data from the data array
x = data[:, 0]
y = data[:, 1]

# Fit the functions to the data using curve_fit
popt3, _ = curve_fit(func3, x, y)
popt2, _ = curve_fit(func2, x, y)
popt1, _ = curve_fit(func1, x, y)

# Plot the data and the fitted functions
plt.plot(x, y, 'o', label='data')
plt.plot(x, func3(x, *popt3), label='3rd-degree polynomial fit')
plt.plot(x, func2(x, *popt2), label='2nd-degree polynomial fit')
plt.plot(x, func1(x, *popt1), label='linear fit')
plt.legend()

plt.xlabel('I, mA')
plt.ylabel('V')
plt.title('Fitted Function')


print("Fitted parameters for 3rd-degree polynomial: ", popt3)
print("Fitted parameters for 2nd-degree polynomial: ", popt2)
print("Fitted parameters for linear function: ", popt1)
plt.show()

# Fitted parameters for 3rd-degree polynomial:  [ 0.62937508 -0.77775178  1.32389591 -0.00803273]
# Fitted parameters for 2nd-degree polynomial:  [0.02944475 1.03610926 0.02053387]
# Fitted parameters for linear function:  [1.06437919 0.01570146]

# Seems that linear function is the best fit as any higher order polynomial
# I = 1.06437919 * V + 0.01570146