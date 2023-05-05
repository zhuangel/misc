# Initialize

## VMCS
- setup vmcs config
- setup vcpu config

# Features

## TSC offset
- describe: guest TSC = host TSC + TSC offset
- controls: MSR_IA32_TSC, MSR_IA32_TSC_ADJUST

## TSC scalling
- describe: guest TSC = host TSC * TSC multiplier + TSC offset
- controls: KVM_SET_TSC_KHZ

## ioeventfd
- describe: register io dev in kvm, vmm poll guest io event via eventfd
- controls: KVM_IOEVENTFD
- details : support PIO, MMIO, VIRTIO CCW

## irqfd
- describe: register irq in kvm, kvm inject irq when vmm notify kvm via eventfd
- controls: KVM_IRQFD, KVM_IRQ_FLAG_RESAMPLE
- details : edge trigger, level trigger(resampler)

## irqfd bypass
- describe: host edge irq inject to guest
- controls: KVM_IRQFD
- details : edge irq, post interrupt, VFIO phy io dev assign to VM
