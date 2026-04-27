from photon import Photon, measure_photon

photon = Photon.create(0, "+")

print(photon)
print("Measure with same basis:", measure_photon(photon, "+"))
print("Measure with different basis:", measure_photon(photon, "x"))