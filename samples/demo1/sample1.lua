rect = exRect:new(10, 10, 20, 20, 20);
print(rect.x, rect.y, rect.width, rect.height, rect.z);
rect.width = 200;
rect.y = rect.y + 20;
rect.x = 2^2;
rect.height = rect.width + rect.x;
rect.z = 8 / 2;
print(rect.x, rect.y, rect.width, rect.height, rect.z);
rect:empty();
print(rect.x, rect.y, rect.width, rect.height, rect.z);
rect:set(231,5346,23,45.22, 52);
print(rect.x, rect.y, rect.width, rect.height, rect.z);
print(rect.S, rect.xyz);