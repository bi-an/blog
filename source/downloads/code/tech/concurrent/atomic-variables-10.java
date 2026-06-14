import java.util.concurrent.atomic.AtomicStampedReference;

public class ABAExample {
    private static AtomicStampedReference<Integer> atomicStampedRef = new AtomicStampedReference<>(0, 0);

    public static void main(String[] args) {
        Thread t1 = new Thread(() -> {
            int stamp = atomicStampedRef.getStamp();
            Integer value = atomicStampedRef.getReference();
            System.out.println("Thread 1 initial value: " + value + ", stamp: " + stamp);
            atomicStampedRef.compareAndSet(value, value + 1, stamp, stamp + 1);
            System.out.println("Thread 1 updated value: " + atomicStampedRef.getReference() + ", stamp: " + atomicStampedRef.getStamp());
        });

        Thread t2 = new Thread(() -> {
            int stamp = atomicStampedRef.getStamp();
            Integer value = atomicStampedRef.getReference();
            System.out.println("Thread 2 initial value: " + value + ", stamp: " + stamp);
            atomicStampedRef.compareAndSet(value, value + 1, stamp, stamp + 1);
            System.out.println("Thread 2 updated value: " + atomicStampedRef.getReference() + ", stamp: " + atomicStampedRef.getStamp());
        });

        t1.start();
        t2.start();
    }
}