// Generated automatically with "cito". Do not edit.

/// Encoder of the "Quite OK Image" (QOI) format.
/// Losslessly compresses an image to a byte array.
public class QOIEncoder
{
	/// Constructs the encoder.
	/// The encoder can be used for several images, one after another.
	public init()
	{
	}

	fileprivate static let headerSize = 14

	fileprivate static let paddingSize = 8

	private var encoded : ArrayRef<UInt8>?

	private var encodedSize : Int = 0

	/// Determines if an image of given size can be encoded.
	/// - parameter width Image width in pixels.
	/// - parameter height Image height in pixels.
	/// - parameter alpha Whether the image has the alpha channel (transparency).
	public static func canEncode(_ width : Int, _ height : Int, _ alpha : Bool) -> Bool
	{
		return width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? 5 : 4)
	}

	/// Encodes the given image.
	/// Returns `true` if encoded successfully.
	/// - parameter width Image width in pixels.
	/// - parameter height Image height in pixels.
	/// - parameter pixels Pixels of the image, top-down, left-to-right.
	/// - parameter alpha `false` specifies that all pixels are opaque. High bytes of `pixels` elements are ignored then.
	/// - parameter linearColorspace Specifies the color space.
	public func encode(_ width : Int, _ height : Int, _ pixels : ArrayRef<Int>?, _ alpha : Bool, _ linearColorspace : Bool) -> Bool
	{
		if pixels === nil || !QOIEncoder.canEncode(width, height, alpha) {
			return false
		}
		let pixelsSize : Int = width * height
		let encoded : ArrayRef<UInt8>? = ArrayRef<UInt8>(repeating: 0, count: 14 + pixelsSize * (alpha ? 5 : 4) + 8)
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
		encoded![13] = linearColorspace ? 1 : 0
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
				if run == 62 || pixelsOffset >= pixelsSize {
					encoded![encodedOffset] = UInt8(191 + run)
					encodedOffset += 1
					run = 0
				}
			}
			else {
				if run > 0 {
					encoded![encodedOffset] = UInt8(191 + run)
					encodedOffset += 1
					run = 0
				}
				let indexOffset : Int = (pixel >> 16 * 3 + pixel >> 8 * 5 + pixel & 63 * 7 + pixel >> 24 * 11) & 63
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
					if (pixel ^ lastPixel) >> 24 != 0 {
						encoded![encodedOffset] = 255
						encoded![encodedOffset + 1] = UInt8(r)
						encoded![encodedOffset + 2] = UInt8(g)
						encoded![encodedOffset + 3] = UInt8(b)
						encoded![encodedOffset + 4] = UInt8(a)
						encodedOffset += 5
					}
					else {
						var dr : Int = r - lastPixel >> 16 & 255
						let dg : Int = g - lastPixel >> 8 & 255
						var db : Int = b - lastPixel & 255
						if dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1 {
							encoded![encodedOffset] = UInt8(106 + dr << 4 + dg << 2 + db)
							encodedOffset += 1
						}
						else {
							dr -= dg
							db -= dg
							if dr >= -8 && dr <= 7 && dg >= -32 && dg <= 31 && db >= -8 && db <= 7 {
								encoded![encodedOffset] = UInt8(160 + dg)
								encoded![encodedOffset + 1] = UInt8(136 + dr << 4 + db)
								encodedOffset += 2
							}
							else {
								encoded![encodedOffset] = 254
								encoded![encodedOffset + 1] = UInt8(r)
								encoded![encodedOffset + 2] = UInt8(g)
								encoded![encodedOffset + 3] = UInt8(b)
								encodedOffset += 4
							}
						}
					}
				}
				lastPixel = pixel
			}
		}
		encoded!.fill(0, encodedOffset, 7)
		encoded![encodedOffset + 8 - 1] = 1
		self.encoded = encoded
		self.encodedSize = encodedOffset + 8
		return true
	}

	/// Returns the encoded file contents.
	/// This method can only be called after `Encode` returned `true`.
	/// The allocated array is usually larger than the encoded data.
	/// Call `GetEncodedSize` to retrieve the number of leading bytes that are significant.
	public func getEncoded() -> ArrayRef<UInt8>?
	{
		return self.encoded
	}

	/// Returns the encoded file length.
	public func getEncodedSize() -> Int
	{
		return self.encodedSize
	}
}

/// Decoder of the "Quite OK Image" (QOI) format.
public class QOIDecoder
{
	/// Constructs the decoder.
	/// The decoder can be used for several images, one after another.
	public init()
	{
	}

	private var width : Int = 0

	private var height : Int = 0

	private var pixels : ArrayRef<Int>?

	private var alpha : Bool = false

	private var linearColorspace : Bool = false

	/// Decodes the given QOI file contents.
	/// Returns `true` if decoded successfully.
	/// - parameter encoded QOI file contents. Only the first `encodedSize` bytes are accessed.
	/// - parameter encodedSize QOI file length.
	public func decode(_ encoded : ArrayRef<UInt8>?, _ ciParamEncodedSize : Int) -> Bool
	{
		var encodedSize : Int = ciParamEncodedSize
		if encoded === nil || encodedSize < 23 || encoded![0] != 113 || encoded![1] != 111 || encoded![2] != 105 || encoded![3] != 102 {
			return false
		}
		let width : Int = Int(encoded![4]) << 24 | Int(encoded![5]) << 16 | Int(encoded![6]) << 8 | Int(encoded![7])
		let height : Int = Int(encoded![8]) << 24 | Int(encoded![9]) << 16 | Int(encoded![10]) << 8 | Int(encoded![11])
		if width <= 0 || height <= 0 || height > 2147483647 / width {
			return false
		}
		switch encoded![12] {
		case 3:
			self.alpha = false
			break
		case 4:
			self.alpha = true
			break
		default:
			return false
		}
		switch encoded![13] {
		case 0:
			self.linearColorspace = false
			break
		case 1:
			self.linearColorspace = true
			break
		default:
			return false
		}
		let pixelsSize : Int = width * height
		let pixels : ArrayRef<Int>? = ArrayRef<Int>(repeating: 0, count: pixelsSize)
		encodedSize -= 8
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
			switch e >> 6 {
			case 0:
				pixel = index[e]
				pixels![pixelsOffset] = pixel
				pixelsOffset += 1
				continue
			case 1:
				pixel = pixel & -16777216 | (pixel + (e >> 4 - 4 - 2) << 16) & 16711680 | (pixel + (e >> 2 & 3 - 2) << 8) & 65280 | (pixel + e & 3 - 2) & 255
				break
			case 2:
				e -= 160
				let rb : Int = Int(encoded![encodedOffset])
				encodedOffset += 1
				pixel = pixel & -16777216 | (pixel + (e + rb >> 4 - 8) << 16) & 16711680 | (pixel + e << 8) & 65280 | (pixel + e + rb & 15 - 8) & 255
				break
			default:
				if e < 254 {
					e -= 191
					if pixelsOffset + e > pixelsSize {
						return false
					}
					pixels!.fill(pixel, pixelsOffset, e)
					pixelsOffset += e
					continue
				}
				if e == 254 {
					pixel = pixel & -16777216 | Int(encoded![encodedOffset]) << 16 | Int(encoded![encodedOffset + 1]) << 8 | Int(encoded![encodedOffset + 2])
					encodedOffset += 3
				}
				else {
					pixel = Int(encoded![encodedOffset + 3]) << 24 | Int(encoded![encodedOffset]) << 16 | Int(encoded![encodedOffset + 1]) << 8 | Int(encoded![encodedOffset + 2])
					encodedOffset += 4
				}
				break
			}
			index[(pixel >> 16 * 3 + pixel >> 8 * 5 + pixel & 63 * 7 + pixel >> 24 * 11) & 63] = pixel
			pixels![pixelsOffset] = index[(pixel >> 16 * 3 + pixel >> 8 * 5 + pixel & 63 * 7 + pixel >> 24 * 11) & 63]
			pixelsOffset += 1
		}
		if encodedOffset != encodedSize {
			return false
		}
		self.width = width
		self.height = height
		self.pixels = pixels
		return true
	}

	/// Returns the width of the decoded image in pixels.
	public func getWidth() -> Int
	{
		return self.width
	}

	/// Returns the height of the decoded image in pixels.
	public func getHeight() -> Int
	{
		return self.height
	}

	/// Returns the pixels of the decoded image, top-down, left-to-right.
	/// Each pixel is a 32-bit integer 0xAARRGGBB.
	public func getPixels() -> ArrayRef<Int>?
	{
		return self.pixels
	}

	/// Returns the information about the alpha channel from the file header.
	public func hasAlpha() -> Bool
	{
		return self.alpha
	}

	/// Returns the color space information from the file header.
	/// `false` = sRGB with linear alpha channel.
	/// `true` = all channels linear.
	public func isLinearColorspace() -> Bool
	{
		return self.linearColorspace
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
