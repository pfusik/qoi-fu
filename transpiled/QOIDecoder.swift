// Generated automatically with "cito". Do not edit.

public class QOIDecoder
{
	public init()
	{
	}

	private var width : Int = 0

	private var height : Int = 0

	private var pixels : ArrayRef<Int>?

	private var alpha : Bool = false

	private var colorspace : Int = 0

	public func decode(_ encoded : ArrayRef<UInt8>?, _ ciParamEncodedSize : Int) -> Bool
	{
		var encodedSize : Int = ciParamEncodedSize
		if encoded === nil || encodedSize < 19 || encoded![0] != 113 || encoded![1] != 111 || encoded![2] != 105 || encoded![3] != 102 {
			return false
		}
		let width : Int = Int(encoded![4]) << 24 | Int(encoded![5]) << 16 | Int(encoded![6]) << 8 | Int(encoded![7])
		let height : Int = Int(encoded![8]) << 24 | Int(encoded![9]) << 16 | Int(encoded![10]) << 8 | Int(encoded![11])
		if width <= 0 || height <= 0 || height > 2147483647 / width {
			return false
		}
		let pixelsSize : Int = width * height
		let pixels : ArrayRef<Int>? = ArrayRef<Int>(repeating: 0, count: pixelsSize)
		encodedSize -= 4
		var encodedOffset : Int = 14
		var index = [Int](repeating: 0, count: 64)
		var pixel : Int = -16777216
		var pixelsOffset : Int = 0
		while pixelsOffset < pixelsSize {
			if encodedOffset >= encodedSize {
				return false
			}
			var e : Int = Int(encoded![encodedOffset])
			encodedOffset += 1
			if e < 128 {
				if e < 64 {
					pixel = index[e]
					pixels![pixelsOffset] = pixel
					pixelsOffset += 1
				}
				else {
					var run : Int = 1 + e & 31
					if e >= 96 {
						run += 32 + Int(encoded![encodedOffset])
						encodedOffset += 1
					}
					if pixelsOffset + run > pixelsSize {
						return false
					}
					pixels!.fill(pixel, pixelsOffset, run)
					pixelsOffset += run
				}
				continue
			}
			else if e < 224 {
				if e < 192 {
					pixel = pixel & -16777216 | (pixel + (e >> 4 - 8 - 2) << 16) & 16711680 | (pixel + (e >> 2 & 3 - 2) << 8) & 65280 | (pixel + e & 3 - 2) & 255
				}
				else {
					let d : Int = Int(encoded![encodedOffset])
					encodedOffset += 1
					pixel = pixel & -16777216 | (pixel + (e - 192 - 16) << 16) & 16711680 | (pixel + (d >> 4 - 8) << 8) & 65280 | (pixel + d & 15 - 8) & 255
				}
			}
			else if e < 240 {
				e = e << 16 | Int(encoded![encodedOffset]) << 8 | Int(encoded![encodedOffset + 1])
				encodedOffset += 2
				pixel = (pixel + (e & 31 - 16) << 24) & -16777216 | (pixel + (e >> 15 - 448 - 16) << 16) & 16711680 | (pixel + (e >> 10 & 31 - 16) << 8) & 65280 | (pixel + e >> 5 & 31 - 16) & 255
			}
			else {
				if e & 8 != 0 {
					pixel = pixel & -16711681 | Int(encoded![encodedOffset]) << 16
					encodedOffset += 1
				}
				if e & 4 != 0 {
					pixel = pixel & -65281 | Int(encoded![encodedOffset]) << 8
					encodedOffset += 1
				}
				if e & 2 != 0 {
					pixel = pixel & -256 | Int(encoded![encodedOffset])
					encodedOffset += 1
				}
				if e & 1 != 0 {
					pixel = pixel & 16777215 | Int(encoded![encodedOffset]) << 24
					encodedOffset += 1
				}
			}
			index[(pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63] = pixel
			pixels![pixelsOffset] = index[(pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63]
			pixelsOffset += 1
		}
		if encodedOffset != encodedSize {
			return false
		}
		self.width = width
		self.height = height
		self.pixels = pixels
		self.alpha = encoded![12] == 4
		self.colorspace = Int(encoded![13])
		return true
	}

	public func getWidth() -> Int
	{
		return self.width
	}

	public func getHeight() -> Int
	{
		return self.height
	}

	public func getPixels() -> ArrayRef<Int>?
	{
		return self.pixels
	}

	public func getAlpha() -> Bool
	{
		return self.alpha
	}

	public func getColorspace() -> Int
	{
		return self.colorspace
	}
}

public class ArrayRef<T> : Sequence
{
	var array : [T]

	init(_ array : [T])
	{
		self.array = array
	}

	init(repeating: T, count: Int)
	{
		self.array = [T](repeating: repeating, count: count)
	}

	init(factory: () -> T, count: Int)
	{
		self.array = (1...count).map({_ in factory() })
	}

	subscript(index: Int) -> T
	{
		get
		{
			return array[index]
		}
		set(value)
		{
			array[index] = value
		}
	}
	subscript(bounds: Range<Int>) -> ArraySlice<T>
	{
		get
		{
			return array[bounds]
		}
		set(value)
		{
			array[bounds] = value
		}
	}

	func fill(_ value: T)
	{
		array = [T](repeating: value, count: array.count)
	}

	func fill(_ value: T, _ startIndex : Int, _ count : Int)
	{
		array[startIndex ..< startIndex + count] = ArraySlice(repeating: value, count: count)
	}

	public func makeIterator() -> IndexingIterator<Array<T>>
	{
		return array.makeIterator()
	}
}
