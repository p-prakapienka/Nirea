// Paths starting with ./ or ../ are relative to the importing file (§12.3).
import ../Common.ni;
import ./Local.ni;

int featureTotal() {
    return commonBase() + localOffset();
}
