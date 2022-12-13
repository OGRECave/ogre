// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

use std::ffi::CStr;
use std::os::raw::c_char;
use std::io::Cursor;
use std::slice;

#[no_mangle]
pub extern "C" fn rs_image_decode(
    input: *const u8,
    input_length: usize,
    ext: *const c_char,
    width: *mut u32,
    height: *mut u32,
    colortype: *mut u32,
    data: *mut *mut u8,
    data_len: *mut usize,
) {
    let slice = unsafe { slice::from_raw_parts(input, input_length) };

    let ext_str = unsafe { CStr::from_ptr(ext).to_str().unwrap() };
    let format = image::ImageFormat::from_extension(ext_str).unwrap();

    let img = image::load_from_memory_with_format(slice, format).unwrap();

    unsafe {
        *width = img.width();
        *height = img.height();
        *colortype = img.color() as u32;
    }

    let mut bytes = img.into_bytes();
    unsafe {
        *data = bytes.as_mut_ptr();
        *data_len = bytes.len();
    }
    std::mem::forget(bytes);
}

#[no_mangle]
pub extern "C" fn rs_image_encode(
    input: *const u8,
    input_length: usize,
    width: u32,
    height: u32,
    t: u32,
    ext: *const c_char,
    data: *mut *mut u8,
    data_len: *mut usize,
) {
    let color = match t {
        0 => image::ColorType::L8,
        1 => image::ColorType::La8,
        2 => image::ColorType::Rgb8,
        3 => image::ColorType::Rgba8,
        _ => return,
    };
    let slice = unsafe { slice::from_raw_parts(input, input_length) };

    let ext_str = unsafe { CStr::from_ptr(ext).to_str().unwrap() };
    let format = image::ImageFormat::from_extension(ext_str).unwrap();

    let mut buf = Cursor::new(Vec::new());

    image::write_buffer_with_format(&mut buf, slice, width, height, color, format).unwrap();
    let mut bytes = buf.into_inner();
    unsafe {
        *data = bytes.as_mut_ptr();
        *data_len = bytes.len();
    }
    std::mem::forget(bytes);
}

#[no_mangle]
pub unsafe extern "C" fn rs_vec_free(ptr: *mut u8, len: usize) {
    Vec::from_raw_parts(ptr, len, len);
}
