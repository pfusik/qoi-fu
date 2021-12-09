// Generated automatically with "cito". Do not edit.

/// QOI color space metadata.
/// Saved in the file header, but doesn't affect encoding or decoding in any way.
public class QOIColorspace
{

	/// sRGBA.
	public static let srgb = 0

	/// sRGB with linear alpha.
	public static let srgbLinearAlpha = 1

	/// Linear RGBA.
	public static let linear = 15
}

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

	fileprivate static let paddingSize = 4

	private var encoded : ArrayRef<UInt8>?

	private var encodedSize : Int = 0

	/// Determines if an image of given size can be encoded.
	/// - parameter width Image width in pixels.
	/// - parameter height Image height in pixels.
	/// - parameter alpha Whether the image has the alpha channel (transparency).
	public static func canEncode(_ width : Int, _ height : Int, _ alpha : Bool) -> Bool
	{
		return width > 0 && height > 0 && height <= 2147483629 / width / (alpha ? 5 : 4)
	}

	/// Encodes the given image.
	/// Returns `true` if encoded successfully.
	/// - parameter width Image width in pixels.
	/// - parameter height Image height in pixels.
	/// - parameter pixels Pixels of the image, top-down, left-to-right.
	/// - parameter alpha `false` specifies that all pixels are opaque. High bytes of `pixels` elements are ignored then.
	/// - parameter colorspace Specifies the color space. See `QOIColorspace`.
	public func encode(_ width : Int, _ height : Int, _ pixels : ArrayRef<Int>?, _ alpha : Bool, _ colorspace : Int) -> Bool
	{
		if pixels === nil || !QOIEncoder.canEncode(width, height, alpha) {
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

	private var colorspace : Int = 0

	/// Decodes the given QOI file contents.
	/// Returns `true` if decoded successfully.
	/// - parameter encoded QOI file contents. Only the first `encodedSize` bytes are accessed.
	/// - parameter encodedSize QOI file length.
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
					var run : Int
					if e < 96 {
						run = e - 63
					}
					else {
						run = 33 + (e - 96) << 8 + Int(encoded![encodedOffset])
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
	public func getAlpha() -> Bool
	{
		return self.alpha
	}

	/// Returns the color space information from the file header.
	/// See `QOIColorspace`.
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
