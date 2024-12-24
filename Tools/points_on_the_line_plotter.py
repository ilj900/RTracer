import matplotlib.pyplot as plt

# Example lists of points
list1 = [0, 1/16, 2/16, 3/16, 4/16, 5/16, 6/16, 7/16, 8/16, 9/16, 10/16, 11/16, 12/16, 13/16, 14/16, 15/16, 1]
list2 = [0, 1/30, 2/30, 3/30, 4/30, 19/30, 20/30, 21/30, 22/30, 23/30, 24/30, 25/30, 26/30, 27/30, 28/30, 29/30, 1]

# Define closer y-coordinates for visualization
y1 = [0] * len(list1)    # All points in list1 at y=0
y2 = [0.3] * len(list2)  # All points in list2 at y=0.3

# Plot the points
plt.scatter(list1, y1, color='red', label='List 1', s=100)  # Red points for list1
plt.scatter(list2, y2, color='blue', label='List 2', s=100)  # Blue points for list2

# Add grid lines for clarity
plt.axhline(0, color='black', linewidth=0.5)    # Line for y=0
plt.axhline(0.3, color='black', linewidth=0.5)  # Line for y=0.3

# Customize the plot
plt.yticks([0, 0.3], ['List 1', 'List 2'])  # Label the y-axis lines
plt.xlabel("Number Line")
plt.ylim(-0.2, 0.5)  # Adjust y-limits to keep the axes compact
plt.legend()
plt.title("Points on Two Closer Parallel Axes")
plt.show()
