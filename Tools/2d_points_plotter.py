import matplotlib.pyplot as plt


def read_points(file_path):
    """
    Reads 2D points from a file.
    Each line in the file should have two numbers separated by a space (x and y coordinates).

    Args:
        file_path (str): Path to the file containing points.

    Returns:
        list: A list of tuples representing points (x, y).
    """
    points = []
    total = 0
    greater = 0
    same_half = 0
    with open(file_path, 'r') as file:
        for line in file:
            try:
                x, y = map(float, line.split())
                total += 1
                if x > y:
                    greater += 1
                if x > 0.5 and y > 0.5:
                    same_half += 1
                if x <= 0.5 and y <= 0.5:
                    same_half += 1
                points.append((x, y))
            except ValueError:
                print(f"Skipping invalid line: {line.strip()}")
    print("Total: " + str(total) + " greater:" + str(greater) + " same_half: " + str(same_half))
    return points


def plot_points(points):
    """
    Plots a list of 2D points.

    Args:
        points (list): A list of tuples representing points (x, y).
    """
    if not points:
        print("No points to plot.")
        return

    x_coords, y_coords = zip(*points)  # Unzip points into x and y coordinates

    plt.figure(figsize=(8, 6))
    plt.scatter(x_coords, y_coords, c='blue', marker='o')
    plt.title("2D Points Plot")
    plt.xlabel("X-axis")
    plt.ylabel("Y-axis")
    plt.grid(True)
    plt.show()


def main():
    file_path = '../data/debug/2d_points.txt'  # Path to your text file containing 3D points
    points = read_points(file_path)
    plot_points(points)


if __name__ == "__main__":
    main()