#![crate_type = "dylib"]

extern crate cpal;

#[no_mangle]
pub extern fn test(value: i32) -> i32 {
    value * 10
}

#[no_mangle]
pub extern fn audioloop() {
    use cpal::Data;
    use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
    let host = cpal::default_host();

    let output_device = host.default_output_device().expect("no output device available");

    let mut supported_configs_range = output_device.supported_output_configs()
        .expect("error while querying configs");
    let supported_config = supported_configs_range.next()
        .expect("no supported config?!")
        .with_max_sample_rate();
    let config = supported_config.into();

    let stream = output_device.build_output_stream(
        &config,
        move |data: &mut [f32], _: &cpal::OutputCallbackInfo| {
            // react to stream events and read or write stream data here.
        },
        move |err| {
            // react to errors here.
        },
    );
}