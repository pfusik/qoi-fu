// QOIPaintDotNet.cs - QOI Paint.NET plugin
//
// Copyright (C) 2021-2022 Piotr Fusik
//
// MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.IO;
using System.Reflection;

using PaintDotNet;
using PaintDotNet.Rendering;

[assembly: AssemblyTitle("Paint.NET Quite OK Image (QOI) plugin")]
[assembly: AssemblyCompany("Piotr Fusik")]
[assembly: AssemblyCopyright("Copyright © 2021-2022")]
[assembly: AssemblyVersion("1.1.2.0")]
[assembly: AssemblyFileVersion("1.1.2.0")]

namespace QOI.PaintDotNet
{
	class QOIFileType : FileType
	{
		public QOIFileType() : base("Quite OK Image", new FileTypeOptions { LoadExtensions = new string[] { ".qoi" }, SaveExtensions = new string[] { ".qoi" } })
		{
		}

		protected override Document OnLoad(Stream input)
		{
			// Read
			long longLength = input.Length;
			if (longLength > 0x7fffffc7) // max byte array length according to https://docs.microsoft.com/en-us/dotnet/api/system.array?view=net-6.0
				throw new Exception("File too long");
			int encodedLength = (int) longLength;
			byte[] encoded = new byte[encodedLength];
			encodedLength = input.Read(encoded, 0, encodedLength);

			// Decode
			QOIDecoder qoi = new QOIDecoder();
			if (!qoi.Decode(encoded, encodedLength))
				throw new Exception("Error decoding QOI");
			int width = qoi.GetWidth();
			int height = qoi.GetHeight();
			int[] pixels = qoi.GetPixels();

			// Pass to Paint.NET
			Document document = new Document(width, height);
			BitmapLayer layer = Layer.CreateBackgroundLayer(width, height);
			Surface surface = layer.Surface;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++)
					surface[x, y] = ColorBgra.FromUInt32((uint) pixels[y * width + x]);
			}
			document.Layers.Add(layer);
			return document;
		}

		protected override void OnSave(Document input, Stream output, SaveConfigToken token, Surface scratchSurface, ProgressEventHandler callback)
		{
			// Fetch from Paint.NET
			input.CreateRenderer().Render(scratchSurface);
			int width = input.Width;
			int height = input.Height;
			int[] pixels = new int[width * height];
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++)
					pixels[y * width + x] = (int) (uint) scratchSurface[x, y];
			}

			// Encode
			QOIEncoder qoi = new QOIEncoder();
			if (!qoi.Encode(width, height, pixels, true, false))
				throw new Exception("Error encoding QOI");

			// Write
			output.Write(qoi.GetEncoded(), 0, qoi.GetEncodedSize());
		}
	}

	public class QOIFileTypeFactory : IFileTypeFactory
	{
		public FileType[] GetFileTypeInstances() => new FileType[] { new QOIFileType() };
	}
}
