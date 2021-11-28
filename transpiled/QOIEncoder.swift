// Generated automatically with "cito". Do not edit.

public class QOIEncoder
{
	public init()
	{
	}

	public static let colorspaceSrgb = 0

	public static let colorspaceSrgbLinearAlpha = 1

	public static let colorspaceLinear = 15

	private static let headerSize = 14

	private var encoded : ArrayRef<UInt8>?

	private var encodedSize : Int = 0

	public func encode(_ width : Int, _ height : Int, _ pixels : ArrayRef<Int>?, _ alpha : Bool, _ colorspace : Int) -> Bool
	{
		if width <= 0 || height <= 0 || height > 429496725 / width || pixels === nil {
			return false
		}
		let pixelsSize : Int = width * height
		let encoded : ArrayRef<UInt8>? = ArrayRef<UInt8>(repeating: 0, count: 14 + pixelsSize * (alpha ? 5 : 4) + 4)
		encoded![0] = 113
		encoded![1] = 111
		encoded![2] = 105
		encoded![3] = 102
		encoded![4] = UInt8(width >> 24)
		encoded![5] = UInt8(width >> 16 & 255)
		encoded![6] = UInt8(width >> 8 & 255)
		encoded![7] = UInt8(width & 255)
		encoded![8] = UInt8(height >> 24)
		encoded![9] = UInt8(height >> 16 & 255)
		encoded![10] = UInt8(height >> 8 & 255)
		encoded![11] = UInt8(height & 255)
		encoded![12] = alpha ? 4 : 3
		encoded![13] = UInt8(colorspace)
		var index = [Int](repeating: 0, count: 64)
		var encodedOffset : Int = 14
		var lastPixel : Int = -16777216
		var run : Int = 0
		var pixelsOffset : Int = 0
		while pixelsOffset < pixelsSize {
			var pixel : Int = pixels![pixelsOffset]
			pixelsOffset += 1
			if !alpha {
				pixel |= -16777216
			}
			if pixel == lastPixel {
				run += 1
			}
			if run > 0 && (pixel != lastPixel || run == 8224 || pixelsOffset >= pixelsSize) {
				if run <= 32 {
					encoded![encodedOffset] = UInt8(64 + run - 1)
					encodedOffset += 1
				}
				else {
					run -= 33
					encoded![encodedOffset] = UInt8(96 + run >> 8)
					encodedOffset += 1
					encoded![encodedOffset] = UInt8(run & 255)
					encodedOffset += 1
				}
				run = 0
			}
			if pixel != lastPixel {
				let indexOffset : Int = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63
				if pixel == index[indexOffset] {
					encoded![encodedOffset] = UInt8(indexOffset)
					encodedOffset += 1
				}
				else {
					index[indexOffset] = pixel
					let r : Int = pixel >> 16 & 255
					let g : Int = pixel >> 8 & 255
					let b : Int = pixel & 255
					let a : Int = pixel >> 24 & 255
					var dr : Int = r - lastPixel >> 16 & 255
					let dg : Int = g - lastPixel >> 8 & 255
					var db : Int = b - lastPixel & 255
					let da : Int = a - lastPixel >> 24 & 255
					if dr >= -16 && dr <= 15 && dg >= -16 && dg <= 15 && db >= -16 && db <= 15 && da >= -16 && da <= 15 {
						if da == 0 && dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1 {
							encoded![encodedOffset] = UInt8(170 + dr << 4 + dg << 2 + db)
							encodedOffset += 1
						}
						else if da == 0 && dg >= -8 && dg <= 7 && db >= -8 && db <= 7 {
							encoded![encodedOffset] = UInt8(208 + dr)
							encodedOffset += 1
							encoded![encodedOffset] = UInt8(136 + dg << 4 + db)
							encodedOffset += 1
						}
						else {
							dr += 16
							encoded![encodedOffset] = UInt8(224 + dr >> 1)
							encodedOffset += 1
							db += 16
							encoded![encodedOffset] = UInt8((dr & 1) << 7 + (dg + 16) << 2 + db >> 3)
							encodedOffset += 1
							encoded![encodedOffset] = UInt8((db & 7) << 5 + da + 16)
							encodedOffset += 1
						}
					}
					else {
						encoded![encodedOffset] = UInt8(240 | (dr != 0 ? 8 : 0) | (dg != 0 ? 4 : 0) | (db != 0 ? 2 : 0) | (da != 0 ? 1 : 0))
						encodedOffset += 1
						if dr != 0 {
							encoded![encodedOffset] = UInt8(r)
							encodedOffset += 1
						}
						if dg != 0 {
							encoded![encodedOffset] = UInt8(g)
							encodedOffset += 1
						}
						if db != 0 {
							encoded![encodedOffset] = UInt8(b)
							encodedOffset += 1
						}
						if da != 0 {
							encoded![encodedOffset] = UInt8(a)
							encodedOffset += 1
						}
					}
				}
				lastPixel = pixel
			}
		}
		encoded!.fill(0, encodedOffset, 4)
		self.encoded = encoded
		self.encodedSize = encodedOffset + 4
		return true
	}

	public func getEncoded() -> ArrayRef<UInt8>?
	{
		return self.encoded
	}

	public func getEncodedSize() -> Int
	{
		return self.encodedSize
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
