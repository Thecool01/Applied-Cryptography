from photon import Photon
from eve import Eve

alice_photon = Photon.create(0, "+")
eve = Eve()

new_photon = eve.intercept_and_resend(alice_photon)

print("Original photon:", alice_photon)
print("Photon after Eve:", new_photon)
print("Eve statistics:", eve.get_statistics())