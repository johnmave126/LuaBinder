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

Class_Begin("superRect", "exRect");

function exRect:new(x, y, width, height, z, length)
	super(x, y, width, height, z);
	__length = length;
	__length = math.max(math.min(__length, 255), 0);
end

function exRect:getV()
	return __width * __height * __length;
end

function exRect:empty()
	super();
	__length = 0;
end

function exRect:set(x, y, width, height, z, length)
	super(x, y, width, height, z);
	__length = length;
	__length = math.max(math.min(__length, 255), 0);
end

function exRect:setlength(length)
	__length = math.max(math.min(length, 255), 0);
end

Class_Attr_r{
	{"volume", "getV"},
	{"length"}
};

Class_Attr_a{
	{"z", "setlength"}
};

Class_End();

rect = superRect:new(10, 10, 20, 20, 20, 100);
print(rect.x, rect.y, rect.width, rect.height, rect.z, rect.length);
rect.width = 200;
rect.y = rect.y + 20;
rect.x = 2^2;
rect.height = rect.width + rect.x;
rect.z = 8 / 2;
rect.length = rect.length * 3
print(rect.x, rect.y, rect.width, rect.height, rect.z, rect.length);
rect:empty();
print(rect.x, rect.y, rect.width, rect.height, rect.z, rect.length);
rect:set(231,5346,23,45.22, 52, 15);
print(rect.x, rect.y, rect.width, rect.height, rect.z, rect.length);
print(rect.S, rect.xyz, rect.volume);