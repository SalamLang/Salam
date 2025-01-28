export enum RuntimeElementAttributeType {
  Any = 'Any',
  AnyNoEmpty = 'AnyNoEmpty',
  Width = 'AnyNoEmpty', // TODO
  String = 'String',
  StringNoEmpty = 'StringNoEmpty',
  Strings = 'Strings',

  Uri = 'Uri',
  Uris = 'Uris',

  Url = 'Url',
  Urls = 'Urls',

  Sizes = 'Sizes',
  Size = 'Size',

  PositiveInt = 'PositiveInt',

  ID = 'ID',
  IDs = 'IDs',

  CSSMedia = 'CSSMedia',

  Coords = 'Coords',

  Color = 'Color',
  Colors = 'Colors',

  Number = 'Number',

  Boolean = 'Boolean',

  Date = 'Date',
  Time = 'Time',
  Times = 'Times',
  DateTime = 'DateTime',

  NumberOr2Numbers = 'NumberOr2Numbers', // aspect-ratio
};
