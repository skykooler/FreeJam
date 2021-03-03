#![crate_type = "dylib"]

extern crate cpal;
use cpal::{Sample, Stream};
use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
use std::sync::{Arc, RwLock};
// use std::time::Duration;
// use std::thread;

const MIDI_FREQS: [f32; 127] = [8.175798915643707, 8.661957218027252, 9.177023997418987, 9.722718241315029, 10.300861153527185,
    10.913382232281371, 11.562325709738575, 12.249857374429665, 12.978271799373285, 13.75, 14.56761754744031,
    15.433853164253879, 16.351597831287414, 17.323914436054505, 18.354047994837973, 19.445436482630058,
    20.60172230705437, 21.826764464562743, 23.12465141947715, 24.49971474885933, 25.95654359874657, 27.5,
    29.13523509488062, 30.867706328507758, 32.70319566257483, 34.64782887210901, 36.70809598967595, 38.890872965260115,
    41.20344461410874, 43.653528929125486, 46.2493028389543, 48.99942949771866, 51.91308719749314, 55.0,
    58.27047018976124, 61.7354126570155, 65.40639132514966, 69.29565774421802, 73.41619197935188, 77.78174593052023,
    82.4068892282175, 87.30705785825097, 92.4986056779086, 97.99885899543733, 103.82617439498628, 110.0,
    116.54094037952248, 123.470825314031, 130.8127826502993, 138.59131548843604, 146.83238395870376, 155.56349186104046,
    164.813778456435, 174.61411571650194, 184.9972113558172, 195.99771799087466, 207.65234878997256, 220.0,
    233.0818807590449, 246.94165062806212, 261.6255653005986, 277.1826309768721, 293.66476791740763, 311.1269837220809,
    329.62755691286986, 349.22823143300394, 369.9944227116344, 391.9954359817492, 415.3046975799452, 440.0,
    466.1637615180898, 493.88330125612424, 523.2511306011972, 554.3652619537442, 587.3295358348153, 622.2539674441618,
    659.2551138257397, 698.4564628660079, 739.9888454232688, 783.9908719634984, 830.6093951598904, 880.0,
    932.3275230361796, 987.7666025122485, 1046.5022612023945, 1108.7305239074883, 1174.6590716696305, 1244.5079348883237,
    1318.5102276514795, 1396.9129257320158, 1479.9776908465376, 1567.9817439269968, 1661.2187903197807, 1760.0,
    1864.6550460723593, 1975.533205024497, 2093.004522404789, 2217.4610478149766, 2349.318143339261, 2489.0158697766474,
    2637.020455302959, 2793.8258514640315, 2959.955381693075, 3135.9634878539937, 3322.4375806395615, 3520.0,
    3729.3100921447212, 3951.0664100489917, 4186.009044809578, 4434.922095629955, 4698.636286678519, 4978.031739553295,
    5274.040910605921, 5587.65170292806, 5919.91076338615, 6271.926975707992, 6644.875161279119, 7040.0,
    7458.6201842894425, 7902.132820097983, 8372.018089619156, 8869.84419125991, 9397.272573357039, 9956.06347910659,
    10548.081821211843, 11175.30340585612, 11839.8215267723];

const MIDI_VOLS: [f32; 128] = [2.21e-06, 2.45e-06, 2.71e-06, 3.01e-06, 3.33e-06, 3.69e-06, 4.09e-06, 4.53e-06, 5.02e-06,
    5.56e-06, 6.16e-06, 6.83e-06, 7.57e-06, 8.38e-06, 9.29e-06, 1.03e-05, 1.14e-05, 1.26e-05, 1.4e-05, 1.55e-05, 1.72e-05,
    1.9e-05, 2.11e-05, 2.34e-05, 2.59e-05, 2.87e-05, 3.18e-05, 3.52e-05, 3.9e-05, 4.32e-05, 4.79e-05, 5.31e-05, 5.88e-05,
    6.52e-05, 7.22e-05, 8e-05, 8.86e-05, 9.82e-05, 0.000109, 0.000121, 0.000134, 0.000148, 0.000164, 0.000182, 0.000201,
    0.000223, 0.000247, 0.000274, 0.000303, 0.000336, 0.000372, 0.000413, 0.000457, 0.000507, 0.000561, 0.000622, 0.000689,
    0.000764, 0.000846, 0.000937, 0.00104, 0.00115, 0.00127, 0.00141, 0.00157, 0.00173, 0.00192, 0.00213, 0.00236, 0.00261,
    0.0029, 0.00321, 0.00355, 0.00394, 0.00436, 0.00483, 0.00536, 0.00594, 0.00658, 0.00729, 0.00807, 0.00894, 0.00991,
    0.011, 0.0122, 0.0135, 0.0149, 0.0165, 0.0183, 0.0203, 0.0225, 0.0249, 0.0276, 0.0306, 0.0339, 0.0376, 0.0416, 0.0461,
    0.0511, 0.0566, 0.0628, 0.0695, 0.077, 0.0854, 0.0946, 0.105, 0.116, 0.129, 0.143, 0.158, 0.175, 0.194, 0.215, 0.238,
    0.264, 0.292, 0.324, 0.359, 0.397, 0.44, 0.488, 0.541, 0.599, 0.664, 0.735, 0.815, 0.903, 1.0];

pub struct AudioSystem {
    stream: Stream,
    current_time: i32,
    timeline_time: i32,
    input: Arc<RwLock<MidiInput>>
}

struct MidiNote {
    note: i8,
    velocity: i8,
    start_time: i32
}

struct MidiInput {
    notes: Vec<MidiNote>
}

#[no_mangle]
pub extern fn test(value: i32) -> i32 {
    value * 10
}

#[no_mangle]
pub extern fn audioloop() -> *mut AudioSystem {

    let stream_input = Arc::new(RwLock::new(MidiInput{ notes: Vec::with_capacity(127) }));

    let host = cpal::default_host();
    let output_device = host.default_output_device().expect("no output device available");
    let mut supported_configs_range = output_device.supported_output_configs()
        .expect("error while querying configs");
    let supported_config = supported_configs_range.next()
        .expect("no supported config?!")
        .with_max_sample_rate();
    let sample_rate = supported_config.sample_rate();
    println!("{:?}", sample_rate);
    let config = supported_config.into();

    let streamarc = Arc::clone(&stream_input);
    let stream = output_device.build_output_stream(
        &config,
        move |data: &mut [f32], _: &cpal::OutputCallbackInfo| {
            let mut n:u32 = 0;
            let mut raw_sample:f32;
            let stream_input = Arc::clone(&stream_input);
            let midi_input = &stream_input.read().unwrap().notes;
            for sample in data.iter_mut() {
                n+=1;
                raw_sample = 0.0;
                // if midi_input.len() > 0 {
                for input in midi_input.iter() {
                    let freq: u32 = MIDI_FREQS[input.note as usize] as u32;
                    let wavelength = sample_rate.0 / freq;
                    if n>wavelength {
                        raw_sample += 0.0;
                    } else {
                        raw_sample += 0.5 * MIDI_VOLS[input.velocity as usize];
                    }
                    if n>wavelength*2 {
                        n=0;
                    }
                }
                *sample = Sample::from(&raw_sample);
            
            }
        },
        move |_err| {
            // react to errors here.
            // rename back to `err` when we actually handle it
        },
    ).unwrap();
    let asys: AudioSystem = AudioSystem {
        stream: stream,
        current_time: 0,
        timeline_time: 0,
        input: streamarc
    };
    asys.stream.play().unwrap();
    let boxed_stream = Box::new(asys);
    Box::into_raw(boxed_stream)
    // thread::sleep(Duration::from_millis(4000))
}

#[no_mangle]
pub extern fn play(ptr: *mut AudioSystem) -> *mut AudioSystem {
    if ptr.is_null() {
        return audioloop();
    }

    let asys = unsafe { Box::from_raw(ptr) };
    asys.stream.play().unwrap();
    Box::into_raw(asys)
}

#[no_mangle]
pub extern fn pause(ptr: *mut AudioSystem) -> *mut AudioSystem {
    if ptr.is_null() {
        return audioloop();
    }

    let asys = unsafe { Box::from_raw(ptr) };
    asys.stream.pause().unwrap();
    Box::into_raw(asys)
}

#[no_mangle]
pub extern fn play_note(ptr: *mut AudioSystem, note: i8, velocity: i8) -> *mut AudioSystem {
    if ptr.is_null() {
        return audioloop();
    }

    let asys = unsafe { Box::from_raw(ptr) };
    asys.input.write().unwrap().notes.push(MidiNote { note: note, velocity: velocity, start_time: asys.current_time});
    Box::into_raw(asys)
}

#[no_mangle]
pub extern fn stop_note(ptr: *mut AudioSystem, note: i8) -> *mut AudioSystem {
    if ptr.is_null() {
        return audioloop();
    }

    let asys = unsafe { Box::from_raw(ptr) };
    {
        let notes = &mut asys.input.write().unwrap().notes;
        if let Some(pos) = notes.iter().position(|x| x.note==note) {
            notes.swap_remove(pos);
        }
    }
    Box::into_raw(asys)
}