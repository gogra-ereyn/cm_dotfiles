#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void insert(int *tree, int *size, int capacity, int value)
{
	if (*size >= capacity) {
		fprintf(stderr, "tree is full\n");
		return;
	}

	tree[(*size)++] = value;
}

int left_child(int index)
{
	return (2 * index) + 1;
}

int right_child(int index)
{
	return (2 * index) + 2;
}

int parent(int index)
{
	return (index - 1) / 2;
}

int is_valid_idx(int index, int size)
{
	return index < size;
}

int get_value(int *tree, int index, int size)
{
	if (is_valid_idx(index, size)) {
		return tree[index];
	}
	fprintf(stderr, "Invalid index\n");
	return -1;
}

void print_tree(int *tree, int size)
{
	for (int i = 0; i < size; i++) {
		printf("%d ", tree[i]);
	}
	printf("\n");
}

void print_diagram(int *tree, int capacity)
{
	if (tree[0] == -1) {
		printf("Empty tree\n");
		return;
	}
	fprintf(stderr, "print diag\n");

	int height = 0;
	int nodeCount = 1;
	while (nodeCount <= capacity) {
		height++;
		nodeCount = 2 * nodeCount + 1;
	}

	for (int level = 0; level < height; level++) {
		int levelStart = (1 << level) - 1;
		int levelEnd = (1 << (level + 1)) - 2;

		int spacing = (1 << (height - level)) - 1;

		for (int i = 0; i < spacing; i++) {
			printf(" ");
		}

		for (int node = levelStart; node <= levelEnd && node < capacity;
		     node++) {
			if (tree[node] != -1) {
				printf("%-2d", tree[node]);
			} else {
				printf("  ");
			}

			for (int i = 0; i < 2 * spacing + 1; i++) {
				printf(" ");
			}
		}
		printf("\n");
	}
}

int tree_height(int *tree, int capacity)
{
	int height = 0;
	for (int i = 0; i < capacity; i++) {
		if (tree[i] != -1) {
			int node_height = 0;
			int temp = i;
			while (temp > 0) {
				temp = parent(temp);
				node_height++;
			}
			if (node_height > height) {
				height = node_height;
			}
		}
	}
	return height + 1;
}

void fill_display(int *tree, int node, int row, int left, int right,
		  char **display, int capacity)
{
	if (node >= capacity || tree[node] == -1)
		return;

	int mid = (left + right) / 2;

	char node_str[4];
	sprintf(node_str, "%d", tree[node]);
	int len = strlen(node_str);

	for (int i = 0; i < len; i++) {
		display[row][mid - len / 2 + i] = node_str[i];
	}

	if (left_child(node) < capacity && tree[left_child(node)] != -1) {
		int left_mid = (left + mid) / 2;
		display[row + 1][left_mid + 1] = '/';
		fill_display(tree, left_child(node), row + 2, left, mid,
			     display, capacity);
	}

	if (right_child(node) < capacity && tree[right_child(node)] != -1) {
		int right_mid = (mid + right) / 2;
		display[row + 1][right_mid - 1] = '\\';
		fill_display(tree, right_child(node), row + 2, mid, right,
			     display, capacity);
	}
}

void print_tree_with_connections(int *tree, int capacity)
{
	if (tree[0] == -1) {
		printf("Empty tree\n");
		return;
	}

	int height = tree_height(tree, capacity);
	int width = (1 << height) * 3;

	char **display = (char **)malloc(height * 2 * sizeof(char *));
	for (int i = 0; i < height * 2; i++) {
		display[i] = (char *)malloc((width + 1) * sizeof(char));
		memset(display[i], ' ', width);
		display[i][width] = '\0';
	}

	fill_display(tree, 0, 0, 0, width, display, capacity);

	for (int i = 0; i < height * 2; i++) {
		int is_empty = 1;
		for (int j = 0; j < width; j++) {
			if (display[i][j] != ' ') {
				is_empty = 0;
				break;
			}
		}
		if (!is_empty) {
			printf("%s\n", display[i]);
		}
	}

	for (int i = 0; i < height * 2; i++) {
		free(display[i]);
	}
	free(display);
}

int main()
{
	int capacity = 16;
	int size = 0;

	int *tree;
	tree = calloc(capacity, sizeof(*tree));

	insert(tree, &size, capacity, 10);
	insert(tree, &size, capacity, 5);
	insert(tree, &size, capacity, 15);
	insert(tree, &size, capacity, 3);
	insert(tree, &size, capacity, 7);

	int root = 0;
	int leftIdx = left_child(root);
	printf("Root: %d\n", get_value(tree, root, size));
	printf("Left child of root: %d\n", get_value(tree, leftIdx, size));

	print_tree_with_connections(tree, capacity);

	free(tree);

	return 0;
}
