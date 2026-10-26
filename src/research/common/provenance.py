"""Explicit source declarations for disputed real-data decoding and geometry."""
import json
from pathlib import Path

from research.common.hashing import digest


PROFILE = 'IEEE_LE_FLOAT32'


def add_real_data_arguments(parser):
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--source-confirmation', type=Path,
                       help='provider/user declaration JSON bound to the raw source SHA-256')
    group.add_argument('--allow-unconfirmed-real', action='store_true',
                       help='explicit sensitivity run; results cannot establish confirmed field-data claims')


def require_inputs(paths):
    missing = [str(Path(p).resolve()) for p in paths if not Path(p).is_file()]
    if missing:
        raise FileNotFoundError('Required inputs are missing:\n' + '\n'.join(missing))


def real_source_provenance(source, confirmation=None, allow_unconfirmed=False):
    """Validate a declaration, never infer confirmation from amplitude plausibility."""
    source = Path(source).resolve()
    require_inputs([source])
    record = dict(source=str(source), source_sha256=digest(source),
                  payload_profile=PROFILE, status='unconfirmed_sensitivity',
                  field_claims_eligible=False, independent_real_sources=1,
                  header_profile_conflict='RRAW header declares IBM; retained decoder assumes IEEE little-endian',
                  offset_units='unconfirmed', amplitude_units='unconfirmed', geometry_description='unconfirmed')
    if confirmation is None:
        if not allow_unconfirmed:
            raise ValueError('Real source encoding/geometry is unconfirmed. Supply --source-confirmation JSON '
                             'or explicitly use --allow-unconfirmed-real for sensitivity diagnostics only.')
        return record
    if allow_unconfirmed:
        raise ValueError('A confirmation and unconfirmed sensitivity mode are mutually exclusive')
    confirmation = Path(confirmation).resolve()
    declaration = json.loads(confirmation.read_text(encoding='utf-8-sig'))
    required = ['source_sha256', 'payload_profile', 'sample_interval_us', 'offset_units',
                'amplitude_units', 'geometry_description', 'confirmed_by', 'evidence_reference']
    missing = [key for key in required if key not in declaration or not str(declaration[key]).strip()
               or declaration[key] is None]
    if missing:
        raise ValueError('Incomplete source confirmation: ' + ', '.join(missing))
    if declaration['source_sha256'] != record['source_sha256']:
        raise ValueError('Source confirmation SHA-256 does not match the raw input')
    if declaration['payload_profile'] != PROFILE or declaration['sample_interval_us'] != 8000:
        raise ValueError('Source declaration does not match the retained IEEE-LE / 8000 us decoder profile')
    if declaration['offset_units'] != 'm':
        raise ValueError('Current physical-coordinate experiments require offsets confirmed in metres')
    record.update(status='declared_confirmed', field_claims_eligible=True,
                  confirmation_path=str(confirmation), confirmation_sha256=digest(confirmation),
                  declaration=declaration,
                  **{key: declaration[key] for key in ['offset_units', 'amplitude_units', 'geometry_description']})
    return record
