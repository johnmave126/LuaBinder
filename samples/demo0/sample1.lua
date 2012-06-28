Class_Begin("exRect", "Rect");

function exRect:new(x, y, width, height, z)
	super(x, y, width, height);
	__z = z;
	__z = math.max(math.min(__z, 255), 0);
end

function exRect:getS()
	return __width * __height;
end

function exRect:getxyz()
	return __x * __y * __z;
end

function exRect:empty()
	super();
	__z = 0;
end

function exRect:set(x, y, width, height, z)
	super(x, y, width, height);
	__z = z;
	__z = math.max(math.min(__z, 255), 0);
end

Class_Attr_r{
	{"S", "getS"},
	{"xyz", "getxyz"},
	{"z"}
};

Class_Attr_a{
	{"z"}
};

Class_End();

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