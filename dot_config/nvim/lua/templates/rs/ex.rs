use std::sync::atomic::{AtomicU64, Ordering};
use std::mem::MaybeUninit;

pub struct MySoA {
    field_u32: Box<[MaybeUninit<u32>]>,
    field_f64: Box<[MaybeUninit<f64>]>,
    end: AtomicU64,
    capacity: u64,
}

impl MySoA {
    pub fn new(capacity: usize) -> Self {
        let mut field_u32 = Vec::with_capacity(capacity);
        let mut field_f64 = Vec::with_capacity(capacity);

        field_u32.resize_with(capacity, MaybeUninit::uninit);
        field_f64.resize_with(capacity, MaybeUninit::uninit);

        Self {
            field_u32: field_u32.into_boxed_slice(),
            field_f64: field_f64.into_boxed_slice(),
            end: AtomicU64::new(0),
            capacity: capacity as u64,
        }
    }

    pub fn push(&self, x: u32, y: f64) -> Result<(), &'static str> {
        let idx = self.end.load(Ordering::Relaxed);

        if idx >= self.capacity {
            return Err("Capacity exceeded");
        }

        unsafe {
            let ptr_u32 = self.field_u32.as_ptr() as *mut MaybeUninit<u32>;
            ptr_u32.add(idx as usize).write(MaybeUninit::new(x));

            let ptr_f64 = self.field_f64.as_ptr() as *mut MaybeUninit<f64>;
            ptr_f64.add(idx as usize).write(MaybeUninit::new(y));
        }

        self.end.store(idx + 1, Ordering::Release);

        Ok(())
    }

    pub fn read_all(&self) -> Vec<(u32, f64)> {
        let end = self.end.load(Ordering::Acquire) as usize;
        let mut result = Vec::with_capacity(end);

        for i in 0..end {
            unsafe {
                let x = self.field_u32[i].assume_init();
                let y = self.field_f64[i].assume_init();
                result.push((x, y));
            }
        }
        result
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Arc;
    use std::thread;

    #[test]
    fn test_concurrent_push_and_read() {
        let soa = Arc::new(MySoA::new(1000));

        let writer_soa = soa.clone();
        let writer = thread::spawn(move || {
            for i in 0..10 {
                writer_soa.push(i, i as f64 + 0.5).unwrap();
            }
        });

        let reader_soa = soa.clone();
        let reader = thread::spawn(move || {
            for _ in 0..5 {
                let snapshot = reader_soa.read_all();
                println!("Reader saw: {:?}", snapshot);
            }
        });

        writer.join().unwrap();
        reader.join().unwrap();

        let final_data = soa.read_all();
        assert_eq!(final_data.len(), 10);
        for (i, (u, f)) in final_data.iter().enumerate() {
            assert_eq!(*u, i as u32);
            assert_eq!(*f, i as f64 + 0.5);
        }
    }
}

